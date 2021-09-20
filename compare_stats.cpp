/*
created by Andrey Kan
andrey.kan@adelaide.edu.au
2021
*/
#include <iostream>
#include <fstream>
#include <vector>


class Customer
{
public:
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int response_time;

    Customer(int par_priority, int par_arrival_time, int par_slots_remaining)
    {
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        response_time = -1;
    }
};

class Stats
{
public:
    int total_wait_0; // for high priority customers
    int total_wait_1; // for regular priority customers
    int total_wait; // for all customers
    int longest_response; // max response time across all customers
    int n_switches;


    Stats();
    bool compute_scheduling_stats(std::ifstream& results_file, std::vector<Customer> &customers);
    void print();
};

void read_customer_info(std::ifstream &in_file, std::vector<Customer> &customers)
{
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        // structure initialization
        // https://en.cppreference.com/w/c/language/struct_initialization
        Customer customer_from_file(priority, arrival_time, slots_requested);
        customers.push_back(customer_from_file);
        customer_id++;
    }
}

Stats::Stats()
{
    total_wait_0 = -1;
    total_wait_1 = -1;
    total_wait = -1;
    longest_response = -1;
    n_switches = -1;
}

// https://www.w3schools.com/cpp/cpp_function_reference.asp
bool Stats::compute_scheduling_stats(std::ifstream& results_file, std::vector<Customer> &customers)
{
    int num_switches = -1; // how many times customers were switched
    int previous_id = -2;
    int max_response = 0; // to keep track of the longest reponse time

    int total_wait_time_0 = 0; // wait time for all priority 0 customers
    int total_wait_time_1 = 0; // wait time for all priority 1 customers

    // priority 0 customers that are currently either playing or waiting
    int playing_or_waiting_0 = 0;
    // priority 1 customers that are currently either playing or waiting
    int playing_or_waiting_1 = 0;

    int last_arrived_id = -1; // most recently arrived customer

    int reference_time = 0;
    int current_time, customer_id;
    while (results_file >> current_time >> customer_id)
    {
        if (current_time != reference_time)
        {
            std::cerr << "Times are incorrect." << std::endl;
            return false;
        }
        while (last_arrived_id + 1 < customers.size())
        {
            // if not all customers have arrived yet
            // identify newly arriving customers at this time slot;
            // the code relies on customers appearing sequentially in the input;
            if (customers[last_arrived_id + 1].arrival_time == current_time)
            {
                // new customer has arrived
                last_arrived_id++;
                if (customers[last_arrived_id].priority == 0)
                {
                    playing_or_waiting_0++;
                }
                else
                {
                    playing_or_waiting_1++;
                }
                // note that more than one customer can arrive at the same time,
                // so check again
                continue;
            }
            break;
        }
        if (customer_id == -1)
        {
            // the machine is not used, increase total wait time
            // for everyone who has arrived but hasn't left yet
            total_wait_time_0 += playing_or_waiting_0;
            total_wait_time_1 += playing_or_waiting_1;
        }
        else
        {
            // some customer is playing
            if ((customer_id < 0) || (customer_id >= customers.size()))
            {
                std::cerr << "Unknown customer." << std::endl;
                return false;
            }
            if (customer_id > last_arrived_id)
            {
                // this relies on customer IDs appearing sequentially in the input
                std::cerr << customer_id << ": scheduled too early." << std::endl;
                return false;
            }

            // everyone who arrived and is not playing is waiting
            total_wait_time_0 += playing_or_waiting_0;
            total_wait_time_1 += playing_or_waiting_1;
            if (customers[customer_id].priority == 0)
            {
                total_wait_time_0--;
            }
            else
            {
                total_wait_time_1--;
            }

            if (customers[customer_id].response_time == -1)
            {
                // this is the first time this customer is playing
                int response_time = current_time - customers[customer_id].arrival_time;
                customers[customer_id].response_time = response_time;
                if (response_time > max_response)
                {
                    max_response = response_time;
                }
            }

            customers[customer_id].slots_remaining--;
            if (customers[customer_id].slots_remaining < 0)
            {
                std::cerr << customer_id << ": redundant runs." << std::endl;
                return false;
            }
            else if (customers[customer_id].slots_remaining == 0)
            {
                // this customer has been satisfied and will be leaving
                if (customers[customer_id].priority == 0)
                {
                    playing_or_waiting_0--;
                }
                else
                {
                    playing_or_waiting_1--;
                }
            }
        }
        if (previous_id != customer_id)
        {
            num_switches++;
            previous_id = customer_id;
        }
        reference_time++;
    }
    if (reference_time == 0)
    {
        std::cerr << "Empty output." << std::endl;
        return false;
    }
    if (playing_or_waiting_0 + playing_or_waiting_1 > 0)
    {
        std::cerr << "Some customers were not satisfied." << std::endl;
        return false;
    }
    if (customer_id != -1)
    {
        std::cerr << "Last line should have no customers (-1 as id)." << std::endl;
        return false;
    }
    total_wait_0 = total_wait_time_0;
    total_wait_1 = total_wait_time_1;
    total_wait = (total_wait_time_0 + total_wait_time_1);
    longest_response = max_response;
    n_switches = num_switches;

    return true;
}

void Stats::print()
{
    std::cout << "total_wait_0 total_wait_1 total_wait ";
    std::cout << "longest_response ";
    std::cout << "n_switches " << std::endl;
    std::cout << total_wait_0 << " ";
    std::cout << total_wait_1 << " ";
    std::cout << total_wait << " ";
    std::cout << longest_response << " ";
    std::cout << n_switches << std::endl;
}

void compare_stat(int baseline_stat, int scheduler_stat)
{
    if (scheduler_stat > baseline_stat)
    {
        std::cout << " increase of " << (scheduler_stat-baseline_stat) << " over baseline (" << baseline_stat << ")" << std::endl;
    }
    else if (scheduler_stat < baseline_stat)
    {
        std::cout << " decrease of " << (baseline_stat-scheduler_stat) << " over baseline (" << baseline_stat << ")" << std::endl;
    }
    else
    {
        std::cout << " equal to baseline (" << baseline_stat << ")" << std::endl;
    }
}

void compare_stats(Stats baseline_stats, Stats scheduler_stats)
{
    std::cout << "total wait time priority 0: " << scheduler_stats.total_wait_0;
    compare_stat(baseline_stats.total_wait_0,scheduler_stats.total_wait_0);
    std::cout << "total wait time priority 1: " << scheduler_stats.total_wait_1;
    compare_stat(baseline_stats.total_wait_1,scheduler_stats.total_wait_1);
    std::cout << "total wait time: " << scheduler_stats.total_wait;
    compare_stat(baseline_stats.total_wait,scheduler_stats.total_wait);
    std::cout << "longest response: " << scheduler_stats.longest_response;
    compare_stat(baseline_stats.longest_response,scheduler_stats.longest_response);
    std::cout << "number of switches: " << scheduler_stats.n_switches;
    compare_stat(baseline_stats.n_switches,scheduler_stats.n_switches);

}

// process command line arguments
// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Provide input and output file names." << std::endl;
        return -1;
    }
    std::ifstream baseline_data_file(argv[1]);
    std::ifstream baseline_results_file(argv[2]);
    std::ifstream scheduler_data_file(argv[1]);
    std::ifstream scheduler_results_file(argv[3]);
    if ((!baseline_data_file) || (!baseline_results_file) || (!scheduler_data_file) || (!scheduler_results_file))
    {
        std::cerr << "Cannot open files." << std::endl;
        return -1;
    }

    // vector: https://www.geeksforgeeks.org/vector-in-cpp-stl/
    std::vector<Customer> baseline_customers; // information about each customer
    std::vector<Customer> scheduler_customers; // information about each customer
    read_customer_info(baseline_data_file, baseline_customers);
    read_customer_info(scheduler_data_file, scheduler_customers);

    Stats baseline_stats, scheduler_stats;
    if (!baseline_stats.compute_scheduling_stats(baseline_results_file, baseline_customers) || (!scheduler_stats.compute_scheduling_stats(scheduler_results_file, scheduler_customers)))
    {
        std::cerr << "INVALID scheduling" << std::endl;
        return -1;
    }

    compare_stats(baseline_stats, scheduler_stats);

    return 0;
}
