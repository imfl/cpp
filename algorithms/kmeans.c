/* 17/10/9 = Mon */

/* For Question 3(A), Assignment 1, Knowledge Discovery in Database */

/* K-Means */

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define NIL -1
#define N 8

typedef struct point Point, *Points, *pPoint;

struct point
{
    double x;
    double y;
};

/* p for Points */
/* n for number of points */
/* m for Points of means */
/* k for number of means */
/* c for clusters */
/* f for flag for change */
/* t for text label */

/* aprint() to print an array of points */
/* cprint() to print an array of clusters */
/* pprint() to print a point */

void kmeans(const Points p, int n, Points m, int k);
void assign(const Points p, int n, const Points m, int k, int* c);
void update(const Points p, int n, Points m, int k, int* c, bool* f);
double dist(Point a, Point b);
void aprint(Points p, int n, char t);
void cprint(int* c, int n, const Points m);
void pprint(Point a);

void kmeans(const Points p, int n, Points m, int k)
{
    int *c = calloc(n, sizeof(int));
    for (int i = 0; i < n; ++i)
        c[i] = NIL;

    bool flag = true;
    int round = 1;

    while (flag) {
        printf(":::::::::::::::::::: Round %d :::::::::::::::::::::\n", round++);
        assign(p, n, m, k, c);
        update(p, n, m, k, c, &flag);
   }

   free(c);
}

void assign(const Points p, int n, const Points m, int k, int* c)
{
    for (int i = 0; i < n; ++i) {
        c[i] = 0;
        double mind = dist(p[i], m[0]);
        for (int j = 1; j < k; ++j) {
            double d = dist(p[i], m[j]);
            if (d < mind) {
                mind = d;
                c[i] = j;
            }
        }
    }
    printf("[Assign]\n"); cprint(c, n, m);
}

void update(const Points p, int n, Points m, int k, int* c, bool* f)
{
    *f = false;
    for (int j = 0; j < k; ++j) {
        double xsum = 0.0, ysum = 0.0;
        int count = 0;
        for (int i = 0; i < n; ++i)
            if (c[i] == j) {
                xsum += p[i].x;
                ysum += p[i].y;
                ++count;
            }
        double xnew = xsum / count, ynew = ysum / count;
        if (xnew == m[j].x && ynew == m[j].y)
            ;
        else {
            *f = true;
            m[j].x = xnew;
            m[j].y = ynew;
        }
    }
    printf("[Update]\n"); aprint(m, k, 'm');
}

double dist(Point a, Point b)
{
    return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

void aprint(Points p, int n, char t)
{
    for (int i = 0; i < n; ++i) {
        printf("%c[%d] = ", t, i+1);
        pprint(p[i]);
        putchar('\n');
    }
}

void cprint(int* c, int n, const Points m)
{
    for (int i = 0; i < n; ++i) {
        printf("c[%d] = m[%d] = ", i+1, c[i]+1);
        pprint(m[c[i]]);
        putchar('\n');
    }
}

void pprint(Point a)
{
    printf("(%.1lf,%.1lf)", a.x, a.y);
}

int main(void)
{
    Point points[N] = { {65,60},{53,60},{65,62},{53,64},{68,63},{51,57},{60,51},{60,80} };

    printf("\n==================== Points ======================\n\n");

    aprint(points, N, 'p');


    printf("\n==================== Question 1 ==================\n\n");

    int k1 = 2;

    Point m1[2] = { {60,51}, {60,80} };

    printf("[Guess]\n"); aprint(m1, k1, 'g');

    kmeans(points, N, m1, k1);


    printf("\n==================== Question 2 ==================\n\n");

    int k2 = 2;

    Point m2[2] = { {53,60}, {65,60} };

    printf("[Guess]\n"); aprint(m2, k2, 'g');

    kmeans(points, N, m2, k2);


    printf("\n==================== Question 3 ==================\n\n");

    int k3 = 3;

    Point m3[3] = { {60,51}, {60,80}, {53,60} };

    printf("[Guess]\n"); aprint(m3, k3, 'g');

    kmeans(points, N, m3, k3);


    printf("\n==================== Question 4 ==================\n\n");

    int k4 = 4;

    Point m4[4] = { {60,51}, {60,80}, {53,60}, {65,60} };

    printf("[Guess]\n"); aprint(m4, k4, 'g');

    kmeans(points, N, m4, k4);

    return 0;
}
