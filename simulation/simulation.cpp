#include "simulation.h"
#include "INIReader.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <tuple>


//static rng for Person
std::mt19937 Person::rng_(std::random_device{}());

// Disease
Disease::Disease(const std::string& name, int duration, double transmissibility):name_(name), duration_(duration), transmissibility_(transmissibility) {}
std::string Disease::name() const{return name_; }
int Disease::duration() const{return duration_; }
double Disease::transmissibility() const{return transmissibility_;}

Person::Person()
    : status_(PersonStatus::SUSCEPTIBLE), days_remaining_(0) {}

void Person::infect(const Disease& disease) {
    if (status_==PersonStatus::SUSCEPTIBLE) {
        status_= PersonStatus::INFECTIOUS;
        days_remaining_=disease.duration();
    }
}
void Person::vaccinate(){
    if (status_==PersonStatus::SUSCEPTIBLE){
        status_=PersonStatus::VACCINATED;
    }
}
void Person::touch(const Person& other, const Disease& disease){
    if(!other.is_infectious()) return;
    if(status_!= PersonStatus::SUSCEPTIBLE) return;

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng_) < disease.transmissibility()){
        infect(disease);
    }
}

void Person::one_more_day(){
    if (status_==PersonStatus::INFECTIOUS){
        if (--days_remaining_<=0){
            status_=PersonStatus::RECOVERED;
            days_remaining_=0;
        }
    }
}

bool Person::is_susceptible() const {return status_==PersonStatus::SUSCEPTIBLE; }
bool Person::is_infectious() const {return status_==PersonStatus::INFECTIOUS; }
bool Person::is_recovered() const {return status_==PersonStatus::RECOVERED; }
bool Person::is_vaccinated() const {return status_==PersonStatus::VACCINATED; }

std::string Person::status_string() const{
    switch (status_){
        case PersonStatus::SUSCEPTIBLE: return "susceptible";
        case PersonStatus::INFECTIOUS: return "infectious";
        case PersonStatus::RECOVERED: return "recovered";
        case PersonStatus::VACCINATED: return "vaccinated";
    }
    return "unknown";
}

//Population

Population::Population(const std::string& name, int size): name_(name), rng_(std::random_device{}()){
    people_.resize(size);
}

void Population::random_infection(const Disease& disease){
    if (people_.empty()) return;
    std::uniform_real_distribution<double> dist(0, people_.size() - 1);
    people_[dist(rng_)].infect(disease);
}

void Population::random_vaccination(double rate){
    if (rate <= 0.0) return;

    std::vector<int> indices(people_.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng_);

    int num =static_cast<int>(rate * people_.size());
    for (int i=0; i < num&& i < (int)people_.size(); i++) {
        people_[indices[i]].vaccinate();
    }
}

void Population::one_more_day(const Disease& disease, int contacts_per_day){

    std::vector<int> sick;
    for(int i=0; i < (int)people_.size(); i++)
        if (people_[i].is_infectious()) sick.push_back(i);

    std::uniform_int_distribution<int> dist(0, people_.size() - 1);
    for(int idx : sick)
        for(int c =0; c < contacts_per_day; c++)
            people_[dist(rng_)].touch(people_[idx], disease);
    
    for (auto& p : people_) p.one_more_day();
}

int Population::count_infectious() const{
    int n=0; for(const auto& p : people_) if (p.is_infectious()) n++; return n;
}

int Population::count_recovered() const{
    int n=0; for(const auto& p : people_) if (p.is_recovered()) n++; return n;
}

int Population::count_vaccinated() const{
    int n=0; for(const auto& p : people_) if (p.is_vaccinated()) n++; return n;
}

int Population::count_susceptible() const{
    int n=0; for(const auto& p : people_) if (p.is_susceptible()) n++; return n;
}

int Population::size() const { 
    return (int)people_.size();
}
std::string Population::name() const {
    return name_;
}

std::string Population::status_string() const{
    std::stringstream ss;
    for (const auto& p : people_){
        if (p.is_susceptible()) ss << '?';
        else if (p.is_infectious()) ss << '+';
        else if (p.is_recovered()) ss << '-';
        else if (p.is_vaccinated()) ss << 'V';
    }
    return ss.str();
}


double Simulation::calc_mean(const std::vector<double>& v) const {
    if (v.empty()) return 0.0;
    return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
}

double Simulation::calc_std(const std::vector<double>& v, double mean) const {
    if (v.size()< 2) return 0.0;
    double sum = 0.0;
    for (double x : v) sum+= (x - mean) * (x - mean);
    return std::sqrt(sum / (v.size() - 1));
}


void Simulation::write_stats_csv(
    const std::string& path,
    const std::vector<RunResult>& results
)
{
    std::vector<double> steps, susc, rec, vax;
    for (const auto& r : results) {
        steps.push_back(r.total_steps);
        susc.push_back(r.susceptible);
        rec.push_back(r.recovered);
        vax.push_back(r.vaccinated);
    }

    double mean_steps = calc_mean(steps);
    double mean_susc = calc_mean(susc);
    double mean_rec = calc_mean(rec);
    double mean_vax = calc_mean(vax);


    std::ofstream f(path);
    f << "key,mean,std\n";
    f << "total_steps,"
      << mean_steps << ","
      << calc_std(steps, mean_steps) << "\n";
    f << "susceptiple_persons,"
      << mean_susc << ","
      << calc_std(susc, mean_susc) << "\n";
    f << "recovered_persons,"
      << mean_rec << ","
      << calc_std(rec, mean_rec) << "\n";
    f << "vaccinated_persons,"
      << mean_vax << ","
      << calc_std(vax, mean_vax) << "\n";

    std::cout << "Written: " << path << std::endl;
}


void Simulation::write_details_csv(
    const std::string& path,
    const std::string& pop_name,
    int run,
    const std::vector<std::tuple<int,int,int,int>>& steps)
{
    std::ofstream f(path, std::ios::app);

    for (const auto& [infectious, recovered, susceptible, vaccinated] : steps) {
        f << pop_name << ","
          << run << ","
          << infectious << ","
          << recovered << ","
          << susceptible << ","
          << vaccinated << "\n";
    }
}

void Simulation::run_herd_immunity_experiment(
    const Disease & disease,
    int pop_size,
    int sim_runs,
    int contacts_per_day,
    const std::string& output_path)
{
    std::cout << "\n=== Herd Immunity Experiment ===" << std::endl;
    std::cout << "Population size" << pop_size << std::endl;
    std::cout << "Disease: " << disease.name()
              << " D=" << disease.duration()
              << " beta=" << disease.transmissibility()
              << " C=" << contacts_per_day << std::endl;

    double R0 = disease.duration() * disease.transmissibility() * contacts_per_day;
    std::cout << "R0 = " << R0 << std::endl;
    std::cout << " Theoretical HIT = "
              << (1.0 - 1.0/R0) * 100.0 << "%" << std::endl;
    

    std::ofstream csv(output_path);
    csv << "vaccination_rate, run, recovered, susceptible, total_steps\n";

    for (int vax_pct = 0; vax_pct <=100; vax_pct += 10){
        double vax_rate = vax_pct / 100.0;

        std::vector<double> recovered_vals;

        for (int run = 0; run < sim_runs; run++) {
            Population pop("HerdTest", pop_size);
            pop.random_vaccination(vax_rate);
            pop.random_infection(disease);

            int steps = 0;
            while (pop.count_infectious() > 0) {
                pop.one_more_day(disease, contacts_per_day);
                steps++;
            }

            int recovered = pop.count_recovered();
            recovered_vals.push_back(recovered);

            csv << vax_pct << ","
                << run     << ","
                << recovered << ","
                << pop.count_susceptible() << ","
                << steps << "\n";
        }

        double mean_rec = calc_mean(recovered_vals);
        std::cout << " Vax=" << vax_pct << "%"
                  << " avg_recovered=" << mean_rec << std::endl;
    }

    std::cout << "Herd immunity data written to: " << output_path << std::endl;
}


Simulation::Simulation(std::string in_file): input_file(in_file) {}

void Simulation::start(){
    std::cout << "Starting simulation..." << input_file << std::endl; 

    INIReader reader(input_file);
    if(reader.ParseError()!=0){
        std::cerr << "Cannot load config file:" << input_file << std::endl;
        return;
    }

    std::string sim_name = reader.Get("global", "simulation_name", "unknown");
    int num_populations = reader.GetInteger("global", "num_populations", 1);
    int sim_runs = reader.GetInteger("global", "simulation_runs", 1);

    std::string disease_name = reader.Get("disease", "name", "unknown");
    int duration = reader.GetInteger("disease", "duration", 5);
    double transmit = reader.GetReal("disease", "transmissibility", 0.2);

    const int CONTACTS_PER_DAY = 6;

    Disease disease(disease_name, duration, transmit);

    std::cout << "Simulation:" << sim_name << std::endl;
    std::cout << "Disease:"  << disease_name << "duration=" <<duration << "transmissibility=" << transmit << std::endl;
    std::cout << "Populations: " << num_populations << std::endl;
    std::cout << "Runs: " << sim_runs << std::endl;


    std::string stats_path ="disease_stats.csv";
    std::string details_path = "disease_details.csv";

    {
        std::ofstream f(details_path);
        f << "name, run, infectious, recovered, susceptible, vaccinated\n";
    }

    std::vector<RunResult> all_results;

    for (int i =1; i <= num_populations; i++) {
        std::string section = "population_" + std::to_string(i);


        std::string pop_name = reader.Get(section, "name", "unknown");
        int pop_size = reader.GetInteger(section, "size", 1000);
        double vax_rate = reader.GetReal(section, "vaccination_rate", 0.0);
        bool patient_0 = reader.GetBoolean(section, "patient_0", false);

        if (pop_name == "unknown") continue;

        std::cout << "\nPopulation: " << pop_name << "size= " << pop_size << "vax_rate=" << vax_rate << "patient_0=" << patient_0 << std::endl;

        for (int run = 0; run < sim_runs; run++){

            Population pop(pop_name,pop_size);
            pop.random_vaccination(vax_rate);
            if (patient_0) pop.random_infection(disease);

            std::vector<std::tuple<int, int, int, int>> step_data;

            int step =0;
            while (pop.count_infectious() > 0){
                step_data.emplace_back(
                    pop.count_infectious(),
                    pop.count_recovered(),
                    pop.count_susceptible(),
                    pop.count_vaccinated());
                pop.one_more_day(disease,6);
                step++;
            }
        
            RunResult result;
            result.total_steps = step;
            result.susceptible = pop.count_susceptible();
            result.recovered = pop.count_recovered();
            result.vaccinated = pop.count_vaccinated();
            all_results.push_back(result);

            write_details_csv(details_path, pop_name, run, step_data);

            std::cout << " Run " << run
                    << "steps" << step
                    << "recovered=" << result.recovered
                    << "susceptible=" << result.susceptible
                    << "vaccinated=" << result.vaccinated
                    << std::endl;
        }
    }
    

    write_stats_csv(stats_path, all_results);

    std::vector<double> steps_v, susc_v, rec_v, vax_v;
    for (const auto& r : all_results) {
        steps_v.push_back(r.total_steps);
        susc_v.push_back(r.susceptible);
        rec_v.push_back(r.recovered);
        vax_v.push_back(r.vaccinated);
    }

    double m_steps = calc_mean(steps_v);
    double m_susc = calc_mean(susc_v);
    double m_rec =calc_mean(rec_v);
    double m_vax = calc_mean(vax_v);

    std::cout << "\n============SUMMARY=============" << std::endl;
    std::cout << "Total steps mean=" << m_steps
              << "std=" << calc_std(steps_v, m_steps) << std::endl;
    std::cout << "Susceptible mean=" << m_susc
              << "std=" << calc_std(susc_v, m_susc) << std::endl;
    std::cout << "Recovered mean=" << m_rec
              << "std=" << calc_std(rec_v, m_rec) << std::endl;
    std::cout << "Vaccinated mean=" << m_vax
              << "std=" << calc_std(vax_v, m_vax) << std::endl;

    run_herd_immunity_experiment(
        disease,
        30000,
        sim_runs,
        CONTACTS_PER_DAY,
        "herd_immunity.csv");
}

