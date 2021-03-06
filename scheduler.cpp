// a1747180, Goldie, Lachlan
// a1740095, Ebbs, Brodie
// a1743106, Downer, Thomas
// hanging on by a pthread
/*
created by Andrey Kan
andrey.kan@adelaide.edu.au
2021
*/
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <algorithm>
#include <functional>

// std is a namespace: https://www.cplusplus.com/doc/oldtutorial/namespaces/
const int PRINT_LOG = 0; // print detailed execution trace

class Customer
{
public:
    std::string name;
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int playing_since;

    Customer(std::string par_name, int par_priority, int par_arrival_time, int par_slots_remaining)
    {
        name = par_name;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        playing_since = -1;
    }
};

class Event
{
public:
    int event_time;
    int customer_id;  // each event involes exactly one customer

    Event(int par_event_time, int par_customer_id)
    {
        event_time = par_event_time;
        customer_id = par_customer_id;
    }
};

void initialize_system(
    std::ifstream &in_file,
    std::deque<Event> &p0_arrivals,
    std::deque<Event> &p1_arrivals,
    std::vector<Customer> &customers)
{
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        Customer customer_from_file(name, priority, arrival_time, slots_requested);
        customers.push_back(customer_from_file);

        // new customer arrival event
        Event arrival_event(arrival_time, customer_id);
        if(priority) {
            //low priority
            p1_arrivals.push_back(arrival_event);
        } else {
            //high priority
            p0_arrivals.push_back(arrival_event);
        }

        customer_id++;
    }
}

void print_state(
    std::ofstream &out_file,
    int current_time,
    int current_id,
    const std::deque<Event> &p0_arrivals,
    const std::deque<Event> &p1_arrivals,
    const std::deque<int> &p0_queue,
    const std::deque<int> &p1_queue)
{
    out_file << current_time << " " << current_id << '\n';
    if (PRINT_LOG == 0)
    {
        return;
    }
    std::cout << current_time << ", " << current_id << "\n p0 arrivals: ";
    for (int i = 0; i < p0_arrivals.size(); i++)
    {
        std::cout << "\t" << p0_arrivals[i].event_time << ", " << p0_arrivals[i].customer_id << ", ";
    }
    std::cout << "\n p1 arrivals: ";
    for (int i = 0; i < p1_arrivals.size(); i++)
    {
        std::cout << "\t" << p1_arrivals[i].event_time << ", " << p1_arrivals[i].customer_id << ", ";
    }
    std::cout << "\n p0 queue: ";
    for (int i = 0; i < p0_queue.size(); i++)
    {
        std::cout << "\t" << p0_queue[i] << ", ";
    }
    std::cout << "\n p1 queue: ";
    for (int i = 0; i < p1_queue.size(); i++)
    {
        std::cout << "\t" << p1_queue[i] << ", ";
    }
    std::cout << '\n';
}

bool compare_customers(int a, int b, std::vector<Customer> &customers, int current_time)
{
    return customers[a].slots_remaining < customers[b].slots_remaining;
}

int find_longest_wait(std::deque<int> &queue, std::vector<Customer> &customers)
{
    int index = -1;

    for (size_t i = 0; i < queue.size(); i++)
    {
        if (customers[i].playing_since == -1)
        {
            if (index == -1)
                index = i;
            else
            {
                index = (customers[i].arrival_time < customers[index].arrival_time) ? i : index;
            }
        }
    }
    
    return index;
}

// process command line arguments
// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Provide input and output file names." << std::endl;
        return -1;
    }
    std::ifstream in_file(argv[1]);
    std::ofstream out_file(argv[2]);
    if ((!in_file) || (!out_file))
    {
        std::cerr << "Cannot open one of the files." << std::endl;
        return -1;
    }

    // deque: https://www.geeksforgeeks.org/deque-cpp-stl/
    // vector: https://www.geeksforgeeks.org/vector-in-cpp-stl/
    std::deque<Event> p0_arrivals; // priority new customer arrivals
    std::deque<Event> p1_arrivals; // new customer arrivals
    std::vector<Customer> customers; // information about each customer

    // read information from file, initialize events queue
    initialize_system(in_file, p0_arrivals, p1_arrivals, customers);

    int current_id = -1; // who is using the machine now, -1 means nobody
    std::deque<int> p0_queue; // priority waiting queue
    std::deque<int> p1_queue; // waiting queue

    std::deque<int> first_serve; //first served queue

    // step by step simulation of each time slot
    bool all_done = false, new_arrivals = false;
    for (int current_time = 0; !all_done; current_time++)
    {
        // welcome newly arrived priority customers to first serve queue
        while (!p0_arrivals.empty() && (current_time == p0_arrivals[0].event_time))
        {
            first_serve.push_back(p0_arrivals[0].customer_id);
            p0_arrivals.pop_front();
            new_arrivals = true;
        }

        // welcome other arrived customers to first serve queue
        while (!p1_arrivals.empty() && (current_time == p1_arrivals[0].event_time))
        {
            first_serve.push_back(p1_arrivals[0].customer_id);
            p1_arrivals.pop_front();
            new_arrivals = true;
        }

        // check if we need to take a customer off the machine
        if (current_id >= 0)
        {
            customers[current_id].slots_remaining--;
            bool taken_off = false;

            if(!first_serve.empty())
            {
                if(first_serve.front() == current_id)
                {
                    // customer has played, take off the first serve queue
                    first_serve.pop_front();

                    if (customers[current_id].slots_remaining > 0)
                    {
                        // customer is not done yet, waiting for the next chance to play
                        if(customers[current_id].priority)
                        {
                            p1_queue.push_back(current_id);
                            using namespace std::placeholders;
                            //sort queue based on remaining time slots
                            std::sort(p1_queue.begin(), p1_queue.end(), std::bind(compare_customers, _1, _2, customers, current_time));
                        }
                        else
                        {
                            p0_queue.push_back(current_id);
                            using namespace std::placeholders;
                            //sort priority queue based on remaining time slots
                            std::sort(p0_queue.begin(), p0_queue.end(), std::bind(compare_customers, _1, _2, customers, current_time));
                        }
                    }

                    current_id = -1;
                    taken_off = true;
                }
            }

            if ((!p0_queue.empty()) && new_arrivals && !taken_off)
            {
                //possible preempt if a new customer has arrived
                if (customers[current_id].slots_remaining > customers[p0_queue.front()].slots_remaining)
                {
                    if (customers[current_id].slots_remaining > 0)
                    {
                        // customer is not done yet, waiting for the next chance to play
                        if(customers[current_id].priority)
                            p1_queue.push_back(current_id);
                        else
                            p0_queue.push_back(current_id);
                    }
                    current_id = -1; // the machine is free now
                    taken_off = true;
                }
            }
            
            if (!p1_queue.empty() && new_arrivals && !taken_off)
            {
                //possible preempt if a new customer has arrived
                if(customers[current_id].slots_remaining > (2-customers[current_id].priority)*customers[p1_queue.front()].slots_remaining)
                {
                    if (customers[current_id].slots_remaining > 0)
                    {
                        // customer is not done yet, waiting for the next chance to play
                        if(customers[current_id].priority)
                            p1_queue.push_back(current_id);
                        else
                            p0_queue.push_back(current_id);
                    }
                    current_id = -1; // the machine is free now
                    taken_off = true;
                }
            }

            if ((customers[current_id].slots_remaining == 0) && !taken_off)
                current_id = -1; //customer has finished
        }

        // if machine is empty, schedule a new customer
        if (current_id == -1)
        {
            if (!first_serve.empty()) //check the first serve queue
            {
                current_id = first_serve.front();
                customers[current_id].playing_since = current_time;               
            }
            else if (!p0_queue.empty() && !p1_queue.empty())// is anyone waiting?
            {
                if(customers[p0_queue.front()].slots_remaining > (2*customers[p1_queue.front()].slots_remaining))
                {
                    current_id = p1_queue.front();
                    p1_queue.pop_front();
                    customers[current_id].playing_since = current_time;
                }
                else
                {
                    current_id = p0_queue.front();
                    p0_queue.pop_front();
                    customers[current_id].playing_since = current_time;
                }
            }
            else if (!p0_queue.empty()) // is anyone waiting in the priority queue?
            {
                current_id = p0_queue.front();
                p0_queue.pop_front();
                customers[current_id].playing_since = current_time;
            }
            else if (!p1_queue.empty())// is anyone else waiting?
            {
                current_id = p1_queue.front();
                p1_queue.pop_front();
                customers[current_id].playing_since = current_time;
            }
        }
        print_state(out_file, current_time, current_id, p0_arrivals, p1_arrivals, p0_queue, p1_queue);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        all_done = (p0_arrivals.empty() && p1_arrivals.empty() && p0_queue.empty() && p1_queue.empty() && (current_id == -1));
        new_arrivals = false;
    }

    return 0;
}
