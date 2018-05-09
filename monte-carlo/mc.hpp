#ifndef MC_HPP
#define MC_HPP

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

#include <algorithm>	// std::max, std::min
#include <cstddef>		// std::size_t
#include <vector>		// std::vector
#include <random>		// std::default_random_engine, std::normal_distribution

enum OptionType 	{call, put};
enum BarrierType 	{out, in};
enum BarrierWay 	{up, down, both};
enum StrikeType 	{floating, fixed};

static constexpr double NULL_STRIKE = -1.0;
inline double positive_of(double x)				{	return x > 0 ? x : 0; }

class Option;
class Path_Dependent_Option;
class Asian_Option;
class Arithmetic_Asian_Option;
class Geometric_Asian_Option;
class LookbackOption;
class BarrierOption;
class Asset;
class Stock;

// TODO: generalize interest rate and discounting into a class
void discount(double &, double);

// TODO: generalize Monte Carlo simulation into a class
bool is_path_dependent(const Option &);
double simulate(const Option &, const Stock &, double, std::default_random_engine &, std::normal_distribution<double> &);
double monte_carlo(std::size_t, int, const Option &, const Stock &, double);

class Option
{
protected:
	OptionType option_type;
	double T;
	mutable double K;
public:
	virtual ~Option() = default;
	Option(OptionType ot, double T, double K) : option_type(ot), T(T), K(K) {}
	OptionType get_type() const 				{ return option_type; }
	double get_expiry() const 					{ return T; }
	double get_strike() const 					{ return K; }
	double get_payoff(double) const;
};

class Path_Dependent_Option : public Option
{
protected:
	std::size_t num_observe;					// number of observation dates
public:
	Path_Dependent_Option(OptionType ot, double T, double K, std::size_t n) : Option(ot, T, K), num_observe(n) {}
	std::size_t get_num_observe() const			{ return num_observe; }
	// hide Option::get_payoff()
	virtual double get_payoff(const std::vector<double> &) const = 0;
};

class Asian_Option : public Path_Dependent_Option
{
protected:
	StrikeType strike_type;
public:
	Asian_Option(OptionType ot, double T, size_t n) : Path_Dependent_Option(ot, T, NULL_STRIKE, n), strike_type(floating) {}
	Asian_Option(OptionType ot, double T, double K, size_t n) : Path_Dependent_Option(ot, T, K, n), strike_type(StrikeType::fixed) {}
	virtual double get_average(const std::vector<double> &) const = 0;
	virtual double get_payoff(const std::vector<double> &) const override;
};

class Arithmetic_Asian_Option : public Asian_Option
{
public:
	using Asian_Option::Asian_Option;
protected:
	virtual double get_average(const std::vector<double> &) const override;
};

class Geometric_Asian_Option : public Asian_Option
{
public:
	using Asian_Option::Asian_Option;
protected:
	virtual double get_average(const std::vector<double> &) const override;
};

class LookbackOption : public Path_Dependent_Option
{
protected:
	StrikeType strike_type;
public:
	// constructor for floating-strike type
	LookbackOption(OptionType ot, double T, size_t n) : Path_Dependent_Option(ot, T, NULL_STRIKE, n), strike_type(floating) {}
	// constructor for fixed-strike type
	LookbackOption(OptionType ot, double T, double K, size_t n) : Path_Dependent_Option(ot, T, K, n), strike_type(StrikeType::fixed) {}
	StrikeType get_strike_type() const			{ return strike_type; }
	virtual double get_payoff(const std::vector<double> &) const override;
};


class BarrierOption : public Path_Dependent_Option
{
protected:
	BarrierType barrier_type;
	BarrierWay barrier_way;
	std::vector<double> barrier;
public:
	BarrierOption(OptionType ot, double T, double K, size_t n, BarrierType bt, BarrierWay bw, double b) : Path_Dependent_Option(ot, T, K, n), barrier_type(bt), barrier_way(bw), barrier{b} {}
	BarrierOption(OptionType ot, double T, double K, size_t n, BarrierType bt, BarrierWay bw, double l, double u) : Path_Dependent_Option(ot, T, K, n), barrier_type(bt), barrier_way(bw), barrier{std::min(l, u), std::max(l, u)} {}
	BarrierType get_barrier_type() const 		{ return barrier_type; }
	virtual double get_payoff(const std::vector<double> &) const override;
private:
	bool is_knocked(double) const;
};

class Asset
{
protected:
	double S_0;		// initial price
	double sigma;	// volatility of returns
public:
	Asset(double S_0, double sigma) : S_0(S_0), sigma(sigma) {}
	virtual ~Asset() = default;
	double get_initial_price() const 			{ return S_0; }
	double get_volatility() const 				{ return sigma; }
};

class Stock : public Asset
{
	double q;		// dividend yield
public:
	Stock(double S_0, double sigma, double q = 0) : Asset(S_0, sigma), q(q) {}
	double get_dividend_yield() const 			{ return q; }
};

#endif