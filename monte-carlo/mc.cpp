// 18/04/15-18 = Sun-Wed

// ::::Aim of design::::

// parallelized Monte Carlo methods to price various options

// ::::Blueprint::::

// The solution is divided into four parts --
// 1. Option: European options such as Vanilla, Barrier, Lookback, and Asian;
// 2. Underlying assets: stock or bonds, on which options are written;
// 3. Interest rate: determinisitc or stochastic, with which we discount future value;
// 4. Monte Carlo engine: simulates one path, generates many paths and collects results;
// 4.1 Simulator & generator: which produce Monte Carlo results
// 4.2 Gatherer: which gathers results, yields convergence tables, and summarizes result

// ::::Notation::::

// [Option Information]
// option_type 				call or put
// T						time to expiry
// K						strike price
// num_observe 				number of observation dates

// [Underlying Information]
// S_0						initial price
// sigma					volatility of returns
// q 						dividend yield

// [Interest Rate Information]
// r 						interest rate (if fixed)

// [Monte Carlo Information]
// N 						number of paths to simulate
// p 						number of processes/threads in parallel computing

#include "mc.hpp"

#include <algorithm>	// std::max_element, std::min_element
#include <cmath>		// std::exp, std::frexp, std::pow
#include <cstddef>		// std::size_t
#include <functional> 	// std::multiplies
#include <iostream>		// std::std::cout, std::std::endl
#include <limits>		// std::numeric_limits
#include <omp.h>		// OMP parallelization
#include <random>		// std::default_random_engine, std::normal_distribution
#include <vector>		// std::vector
#include <stdexcept>	// std::runtime_error

double Option::get_payoff(double S) const
{
	switch (option_type)
	{
		case call:	return positive_of(S - K);
		case put:	return positive_of(K - S);
		default: 	throw std::runtime_error("Unknown OptionType!");
	}
}

double Asian_Option::get_payoff(const std::vector<double> & S) const
{
		#ifdef DEBUG
			std::clog << "DEBUG: get_payoff() for asian" << std::endl;
		#endif
		if (S.empty()) 
			throw std::runtime_error("Empty std::vector in get_payoff()!");
		switch (strike_type) {
			// call_float = (S_T - S_avg)+
			// put_float = (S_avg - S_T)+
			case floating: {
				K = get_average(S);
				return Option::get_payoff(S.back());
			}
			// call_fix = (S_avg - K)+
			// put_fix = (K - S_avg)+
			case StrikeType::fixed: {
				return Option::get_payoff(get_average(S));
			}
			default: throw std::runtime_error("Unknown StrikeType!");
		}
	}


double Arithmetic_Asian_Option::get_average(const std::vector<double> & S) const
{
	return std::accumulate(S.cbegin(), S.cend(), 0.0) / S.size();
}

double Geometric_Asian_Option::get_average(const std::vector<double> & S) const
{
	// Given the number 123.45, frexp() makes 0.964453 * 2 ^ 7
	double mantissa = 1.0;
    long long exponent = 0L;
    double inverse = 1.0 / S.size();
	for (double x : S) {
		int i {};
		mantissa *= std::frexp(x, &i);
		exponent += i;
	}
    return std::pow(std::numeric_limits<double>::radix, exponent * inverse) * std::pow(mantissa, inverse);
	// the above implementation is better than (because less likely to overflow):
	// return pow(std::accumulate(S.cbegin(), S.cend(), 1.0, std::multiplies<double>()), 1.0 / S.size());
}

double LookbackOption::get_payoff(const std::vector<double> & S) const
{
	#ifdef DEBUG
		std::clog << "DEBUG: get_payoff() for lookback" << std::endl;
	#endif
	if (S.empty()) 
		throw std::runtime_error("Empty std::vector in get_payoff()!");
	switch (strike_type) {
		// call_float = positive_of(S_T - S_min)
		// put_float = positive_of(S_max - S_T)
		case floating: {
			switch (option_type) {
				case call:	K = *std::min_element(S.cbegin(), S.cend()); break;
				case put:	K = *std::max_element(S.cbegin(), S.cend()); break;
				default: 	throw std::runtime_error("Unknown OptionType!");
			}
			return Option::get_payoff(S.back());
		}
		// call_fix = positive_of(S_max - K)
		// put_fix = positive_of(K - S_min)
		case StrikeType::fixed: {
			switch (option_type) {
				case call:	return Option::get_payoff(*std::max_element(S.cbegin(), S.cend()));
				case put:	return Option::get_payoff(*std::min_element(S.cbegin(), S.cend()));
				default: 	throw std::runtime_error("Unknown OptionType!");
			}
		}
		default: throw std::runtime_error("Unknown StrikeType!");
	}
}

double BarrierOption::get_payoff(const std::vector<double> & S) const
{
	#ifdef DEBUG
		std::clog << "DEBUG: get_payoff() for barrier" << std::endl;
	#endif
	if (S.empty()) 
		throw std::runtime_error("Empty std::vector in get_payoff()!");
	switch (barrier_type) {
		case out: {
			for (const auto & s : S)
			if (is_knocked(s))
				return 0;
			return Option::get_payoff(S.back());
		}
		case in: {
			for (const auto & s : S)
			if (is_knocked(s))
				return Option::get_payoff(S.back());
			return 0;
		}
		default: throw std::runtime_error("Unknown BarrierType!");
	}			
}

bool BarrierOption::is_knocked(double S) const
{
	switch (barrier_way) {
		case up: 	return S >= barrier.front();
		case down: 	return S <= barrier.front();
		case both:	return S <= barrier.front() || S >= barrier.back();
		default:	throw std::runtime_error("Unknown BarrierWay!");
	}
}


// interest rate information
constexpr double r = 0.05;

void discount(double & val, double T)
{
	val /= std::exp(r * T);
}

bool is_path_dependent(const Option & opt)
{
	return dynamic_cast<const Path_Dependent_Option *>(&opt) != nullptr;
}

double simulate(const Option & opt, const Stock & stk, double r, std::default_random_engine & engine, std::normal_distribution<double> & norm_dist)
{
	double T = opt.get_expiry();
	double S_0 = stk.get_initial_price();
	double sigma = stk.get_volatility();
	double q = stk.get_dividend_yield();
	if (is_path_dependent(opt)) {
		auto f = [&](const Path_Dependent_Option & opt) -> double
		{
			int n = opt.get_num_observe();
			std::vector<double> S;
			double dt = T / n;
			double S_t = S_0;
			double z = 0;
			for (int i = 0; i != n; ++i) {
				z = norm_dist(engine);
				S_t *= exp((r - q - 0.5 * sigma * sigma) * dt + sigma * sqrt(dt) * z);
				S.push_back(S_t);
			}
			return opt.get_payoff(S);
		};
		return f(dynamic_cast<const Path_Dependent_Option &>(opt));
	}
	else {
		double z = norm_dist(engine);
		double S_T = S_0 * exp((r - q - 0.5 * sigma * sigma) * T + sigma * sqrt(T) * z);
		return opt.get_payoff(S_T);
	}
}

double monte_carlo(std::size_t N, int p, const Option & opt, const Stock & stk, double r)
{
	std::vector<double> records(N, 0);	
	std::default_random_engine engine(0);				// set seed = 0 temporarily
	std::normal_distribution<double> norm_dist(0, 1);
	double result = 0;
	int n = N / p;
	#pragma omp parallel num_threads(p) reduction(+: result)
	{
		int my_rank = omp_get_thread_num();
		int my_begin = my_rank * n;
		int my_end = (my_rank == p - 1 ? N : my_begin + n);
		while (my_begin != my_end) {
			double run = simulate(opt, stk, r, engine, norm_dist);
			records[my_begin++] = run;
			result += run;
		}
	}
	result /= N;
	discount(result, opt.get_expiry());
	return result;
}

int main(int argc, char ** argv)
{

	// option information
	double T = 1;
	double K = 100;
	double U = 120;		// upper bound for up-and-out barrier option
	double L = 80;		// lower bound for up-and-out barrier option
	double S_0 = 100;
	double sigma = 0.1;
	// Monte Carlo information
	std::size_t N = 1'000'000;
	int p = 4;
	double n = 10;		// number of observation dates
	if (argc != 1)
		p = atoi(argv[1]);
	Stock stk(S_0, sigma);
	std::cout << monte_carlo(N, p, Option(call, T, K), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Option(put, T, K), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, BarrierOption(call, T, K, n, out, up, U), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, BarrierOption(put, T, K, n, out, down, L), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, LookbackOption(call, T, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, LookbackOption(put, T, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, LookbackOption(call, T, K, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, LookbackOption(put, T, K, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Arithmetic_Asian_Option(call, T, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Arithmetic_Asian_Option(put, T, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Arithmetic_Asian_Option(call, T, K, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Arithmetic_Asian_Option(put, T, K, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Geometric_Asian_Option(call, T, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Geometric_Asian_Option(put, T, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Geometric_Asian_Option(call, T, K, n), stk, r) << std::endl;
	std::cout << monte_carlo(N, p, Geometric_Asian_Option(put, T, K, n), stk, r) << std::endl;
	
	return 0;
}