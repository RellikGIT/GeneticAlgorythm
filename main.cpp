#include <chrono>
#include <iostream>
#include <malloc.h>
#include <fstream>
#include <random>
using namespace std;

const int population_size = 60; //Количество хромосом в поколении
const int crossover_prob = 100;
const int mutation_prob = 10;   //Вероятности
const int lifetime = 2000;      //Итераций

int N;                          //Количество точек, а ещё длина массива с маршрутом
mt19937 gen(unsigned(time(nullptr)));
struct Chromosome
{
    int* one_chromosome; //Массив - маршрут
    void print()        //Было для тестов - не нужно
    {
        for (int i = 0; i < N; ++i) cout << one_chromosome[i] << " ";
        cout << endl;
    }
};
Chromosome create_random_chromosome()
{
    Chromosome c;
    c.one_chromosome = new int[N];
    for (int i = 0; i < N; ++i) c.one_chromosome[i] = i;

    //Шафл городов, повторить досконально
    for (int i = N - 1; i > 0; --i)
    {
        uniform_int_distribution<int> uid(1, i);
        swap(c.one_chromosome[i], c.one_chromosome[uid(gen)]);
    }
    return c;
}
double fitness(double** source, Chromosome c)//1 / длина маршрута
{

    int value = 0;
    for (int i = 0; i < N - 1; ++i)
        value += source[c.one_chromosome[i]][c.one_chromosome[i + 1]];
    return 1.0 / value;
}
int value_f(double** source, Chromosome c) //просто длина маршрута(в том числе не полного)
{
    int value = 0;
    for (int i = 0; i < N - 1; ++i)
        if (c.one_chromosome[i] == -1) break;
        else
            value += source[c.one_chromosome[i]][c.one_chromosome[i + 1]];
    return value;
}
double fitness_sum(double** source, Chromosome* population, int last)   //Сумма фитнесов до last включительно
{
    double ans = 0;
    for (int i = 0; i <= last; ++i)
        ans += fitness(source, population[i]);
    return ans;
}
void RWS(double** source, Chromosome* population, double* points, int keep_size)//Выбор лучших хромосом
{
    Chromosome* keep = new Chromosome [keep_size];
    for (int i = 0; i < keep_size; ++i) keep[i].one_chromosome = new int[N];
    int ind = 0;

    for (int in = 0; in < keep_size; ++in)
    {
        double p = points[in];
        int i = 0;
        //Поиск точки с таким фитнессом, чтобы сумма была >=p
        while (fitness_sum(source,population, i) < p)
            ++i;
        //Добавление в лучшие
        keep[ind].one_chromosome = population[i].one_chromosome;
        ind++;
    }

    for (int i = 0; i < keep_size; ++i)
        population[i].one_chromosome = keep[i].one_chromosome;
}
void Selection (double** source, Chromosome* population) //Алгоритм SUS
{
    //Сумма всей популяции
    double F = fitness_sum(source, population, population_size - 1);
    //Количество отбираемых
    int n = 2 * population_size / 3;
    //Шаг
    double P = ((double)F) / n;
    //Точка начала
    double start = (double(gen()) - gen.min()) / (gen.max() - gen.min()) * P;

    //Все точки
    double* points = new double [n];
    for (int i = 0; i < n; ++i)
    {
        //Шагаем от старта по P
        points[i] = start + i*P;

    }
    //Соотносим дабловским fitnessaм хромосомы
    RWS(source, population, points, n);
}


int search_next(Chromosome c, int value) //Поиск индекса следующего за индексом элемента value
{
    for (int i = 0; i < N; ++i)
        if (c.one_chromosome[i] == value) return i + 1;
    return N;
}
int cheapest(double** source, int from, Chromosome& c, int ind) //Поиск кратчайшего пути из from
{
    //В город не включенный в маршрут c до ind включительно
    int cheapest_cost = 999999;
    int cheapest = 0;
    bool break_flag = false;
    for (int i = 0; i < N; ++i)
    {
        //Проверка каждого города есть ли он в маршруте
        break_flag = false;
        for (int j = 0; j <= ind; ++j)
            if (c.one_chromosome[j] == i)
            {
                break_flag = true;
                break;
            }
        //Если города в маршруте нет, то сравниваем с самым дешевым переходом.
        if (!(break_flag))
            if (source[from][i] < cheapest_cost)
            {
                cheapest_cost = source[from][i];
                cheapest = i;
            }
    }
    //Вовзращаем индекс самого дешевого перехода
    return cheapest;
}
bool present(Chromosome& c, int ind, int value)
{
    //Проверка есть ли город value в маршруте c до ind (да, я не включил в функцию выше потому что написал это позже)
    for (int j = 1; j <= ind; ++j)
        if (c.one_chromosome[j] == value)
            return true;
    return false;
}
void Greedy(double** source, Chromosome& p1, Chromosome& p2, Chromosome& c)
{
    //Под городами подразумеваются точки, везде и далее

    c.one_chromosome[0] = p1.one_chromosome[0];
    //find next legit(not in route) city in parent 1
    //if there is not, choose cheapest from 1 to one of availiable
    //if there is, same for city 2
    //if both are found, choose best
    for (int i = 1; i < N; ++i)
    {
        //Прошлый город
        int prev_gen = c.one_chromosome[i - 1];
        //Индекс следующего
        int next_ind = search_next(p1, prev_gen);
        bool select_cheapest = true;
        //Если следующего в маршруте нет(конец маршрута)
        if (next_ind != N)
        {
            //Иначе получаем номер города
            int first_candidate = p1.one_chromosome[next_ind];
            //Если города нет в маршруте
            if (!present(c, i - 1, first_candidate))
            {
                //То же самое для 2 родителя
                next_ind = search_next(p2, prev_gen);
                if (next_ind != N)
                {
                    int second_candidate = p2.one_chromosome[next_ind];
                    if (!present(c, i - 1, second_candidate))
                    {
                        //Если все условия выполнены выбираем лучший из 2 переходов
                        select_cheapest = false;

                        if (source[prev_gen][first_candidate] < source[prev_gen][second_candidate])
                            c.one_chromosome[i] = first_candidate;
                        else
                            c.one_chromosome[i] = second_candidate;
                    }
                }
            }
        }
        //Если любое нарушено выбираем самый дешевый из доступных
        if (select_cheapest)
            c.one_chromosome[i] = cheapest(source, prev_gen, c, i - 1);
    }
}

void Crossover(double** source, Chromosome* population)
{
    for (int i = 0; i < 2 *population_size / 3; i += 2)
        Greedy(source, population[i], population[i + 1], population[i / 2 + 2 *population_size / 3]);
}

void mutator(Chromosome& c)
{
    //Свапаем 2 любых пункта в маршруте. Не свапаем одинаковые. Не свапаем 0 ни с каким
    uniform_int_distribution<int> uid(1, N - 1);
    int a;
    a = uid(gen);

    int b;
    do b = uid(gen); while (b == a);
    swap(c.one_chromosome[a], c.one_chromosome[b]);
}
void Mutation(Chromosome* population)
{
    uniform_int_distribution<int> uid(1, 100);
    for (int i = 0; i < population_size; ++i)
    {
        //С шансом mutation_prob вызываем mutator для каждой хромосомы
        int chance = uid(gen);
        if (chance <= mutation_prob)
            mutator(population[i]);
    }
}
Chromosome best(double** source, Chromosome* population)
{
    //Выбор лучшей хромосомы в популяции по fitness
    int best_i = 0;
    for (int i = 1; i < population_size; ++i)
        if (fitness(source, population[i]) > fitness(source, population[best_i]))
            best_i = i;
    return population[best_i];
}

Chromosome GA(double ** matrix)
{
    //Главная функция
    //Лучшая хромосома
    Chromosome best_overall;
    best_overall.one_chromosome = new int[N];
    for (int i = 0; i < N; ++i) best_overall.one_chromosome[i] = i;

    //Популяция
    Chromosome* population = new Chromosome[population_size];
    for (int i = 0; i < population_size; ++i) population[i] = create_random_chromosome();

    for (int i = 0; i < lifetime; ++i)
    {
        Selection(matrix, population);
        Crossover(matrix,  population);
        Mutation(population);

        //Сравнение лучшей из этого поколения с лучшей за всё время
        double best_overall_f = fitness(matrix,best_overall);
        Chromosome challenger = best(matrix, population);
        double best_current_f = fitness(matrix, challenger);

        //Замена лучшей за всё время
        if(best_current_f > best_overall_f)
            for (int j = 0; j < N; ++j)
                best_overall.one_chromosome[j] = challenger.one_chromosome[j];
    }
    return best_overall;
}

// точка входа в программу
int main () {
	setlocale (LC_ALL, "Russian");

	double **D = nullptr;
	N = 0;

    ifstream is("in.txt");

// инициализация матрицы расстояний
	is >> N;
	D = (double **) malloc (sizeof (double *) * N);
	for (int i=0; i<N; ++i) {
		D[i] = (double *) malloc (sizeof (double) * N);
		for (int j=0; j<N; ++j) is >> D[i][j];
	}
    D[0][1] = 999999999;
    D[1][0] = 999999999;
    ofstream os("out.txt");

    //Получение лучшей хромосомы
    Chromosome sol = GA(D);

    Chromosome way1;
    way1.one_chromosome = new int[N];
    for (int i = 0; i < N; ++i) way1.one_chromosome[i] = -1;

    Chromosome way2;
    way2.one_chromosome = new int[N];
    for (int i = 0; i < N; ++i) way2.one_chromosome[i] = -1;

    int ind = 0;
    while (sol.one_chromosome[ind] != 1)
    {
        way1.one_chromosome[ind] = sol.one_chromosome[ind];
        ++ind;
    }

    for (int i = 0; i < N - ind; ++i)
        way2.one_chromosome[i] = sol.one_chromosome[i + ind];


    //Сверху получение 2 путей, снизу вывод
    for (int i = 0; i < N; ++i)
        if (way1.one_chromosome[i] == -1) break;
        else
            cout << way1.one_chromosome[i] + 1 << " ";
    cout << endl << endl << value_f(D, way1) << endl << endl << endl;

    for (int i = 0; i < N; ++i)
        if (way2.one_chromosome[i] == -1) break;
        else
            cout << way2.one_chromosome[i] + 1  << " ";
    cout << endl << endl << value_f(D, way2) << endl << endl << endl;

    cout <<value_f(D, way1) + value_f(D, way2) << endl;

    cout << "FIND MATCHES!\n";
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            if (sol.one_chromosome[i] > sol.one_chromosome[j]) swap(sol.one_chromosome[i],sol.one_chromosome[j]);
    for (int i = 0; i < N - 1; ++i)
    {
        if (sol.one_chromosome[i] == -1) break;
        else
            if (sol.one_chromosome[i] == sol.one_chromosome[i + 1])
                cout << sol.one_chromosome[i] + 1 << " ";
    }
	return 0;
}
