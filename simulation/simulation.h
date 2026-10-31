#ifndef HPC_DISEASE_SIMULATION_H_
#define HPC_DISEASE_SIMULATION_H_
#endif // HPC_DISEASE_SIMULATION_H_

#include <string>
#include <vector>
#include <random>

class Disease {
public:
  Disease(const std::string& name, int duration, double transmissibility);
  std::string name() const;
  int duration() const;
  double transmissibility() const;
private:
  std::string name_;
  int duration_;
  double transmissibility_;
};


enum class PersonStatus{
  SUSCEPTIBLE,
  INFECTIOUS,
  RECOVERED,
  VACCINATED
};

class Person {
public:
  Person();
  void infect(const Disease & disease);
  void vaccinate();
  void touch(const Person& other, const Disease & disease);
  void one_more_day();

  bool is_susceptible() const;
  bool is_infectious() const;
  bool is_recovered() const;
  bool is_vaccinated() const;

  std::string status_string() const;
private:
  PersonStatus status_;
  int days_remaining_;
  static std::mt19937 rng_;
};



class Population {
public:
  Population(const std::string& name, int size);
  void random_infection(const Disease& disease);
  void random_vaccination(double rate);
  void one_more_day(const Disease& disease, int contacts_per_day);

  int count_susceptible() const;
  int count_infectious() const;
  int count_recovered() const;
  int count_vaccinated() const;
  int size() const;

  std::string name() const;
  std::string status_string() const;

private:
  std::string name_;
  std::vector<Person> people_;
  std::mt19937 rng_;
};

struct RunResult {
  int total_steps;
  int susceptible;
  int recovered;
  int vaccinated;
};

struct PopulationConfig {
  std::string name;
  int size;
  double vaccination_rate;
  bool patient_0;
};

class Simulation {
public:
  Simulation(std::string in_file = "disease_in.ini");
  void start();

private:
  std::string input_file;
  
  void write_stats_csv(
    const std::string& path,
    const std::vector<RunResult>& results);
  
  void write_details_csv(
    const std::string& path,
    const std::string& pop_name,
    int run,
    const std::vector<std::tuple<int,int,int,int>>& steps);
  
  double calc_mean(const std::vector<double>& v) const;
  double calc_std(const std::vector<double>& v, double mean) const;

  void run_herd_immunity_experiment(
    const Disease & disease,
    int pop_size,
    int sim_runs,
    int contacts_per_day,
    const std::string & output_path
  );

};
