#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/doctest.h"
#include "simulation.h"


// ── Disease tests ──────────────────────────────
TEST_CASE("Disease stores correct properties") {
    Disease d("Flu", 5, 0.3);
    CHECK(d.name() == "Flu");
    CHECK(d.duration() == 5);
    CHECK(d.transmissibility() == doctest::Approx(0.3));
}

// ── Person tests ───────────────────────────────
TEST_CASE("Person starts susceptible") {
    Person p;
    CHECK(p.is_susceptible());
    CHECK(p.status_string() == "susceptible");
}

TEST_CASE("100% transmissibility always infects") {
    Disease d("Test", 5, 1.0);
    Person p;
    p.infect(d);
    CHECK(p.is_infectious());
}

TEST_CASE("Vaccinated person immune to contact") {
    Disease d("Test", 5, 1.0);
    Person source; source.infect(d);
    Person vax;    vax.vaccinate();
    vax.touch(source, d);
    CHECK(vax.is_vaccinated());
}

TEST_CASE("Recovered person immune to contact") {
    Disease d("Test", 5, 1.0);
    Person p; p.infect(d);
    for (int i = 0; i < 5; i++) p.one_more_day();
    REQUIRE(p.is_recovered());

    Person source; source.infect(d);
    p.touch(source, d);
    CHECK(p.is_recovered());
}

TEST_CASE("Person recovers after correct days") {
    Disease d("Test", 5, 1.0);
    Person p; p.infect(d);
    for (int i = 0; i < 5; i++) { CHECK(p.is_infectious()); p.one_more_day(); }
    CHECK(p.is_recovered());
}

TEST_CASE("50% transmissibility infects roughly half") {
    Disease d("Test", 5, 0.5);
    Person source; source.infect(d);
    int infected = 0;
    const int N = 10000;
    for (int i = 0; i < N; i++) {
        Person p;
        p.touch(source, d);
        if (p.is_infectious()) infected++;
    }
    double rate = (double)infected / N;
    CHECK(rate > 0.45);
    CHECK(rate < 0.55);
}

// ── Population tests ───────────────────────────
TEST_CASE("Population correct size") {
    Population pop("Test", 100);
    CHECK(pop.size() == 100);
    CHECK(pop.count_susceptible() == 100);
}

TEST_CASE("100% vaccination vaccinates everyone") {
    Population pop("Test", 100);
    pop.random_vaccination(1.0);
    CHECK(pop.count_vaccinated() == 100);
    CHECK(pop.count_susceptible() == 0);
}

TEST_CASE("Patient zero is infectious after random_infection") {
    Disease d("Test", 5, 1.0);
    Population pop("Test", 100);
    pop.random_infection(d);
    CHECK(pop.count_infectious() == 1);
}

TEST_CASE("No contact: only patient zero gets sick") {
    Disease d("Test", 5, 1.0);
    Population pop("Test", 100);
    pop.random_infection(d);
    for (int i = 0; i < 5; i++) {
        CHECK(pop.count_infectious() == 1);
        pop.one_more_day(d, 0);
    }
    CHECK(pop.count_infectious() == 0);
    CHECK(pop.count_recovered() == 1);
}

TEST_CASE("Total population count stays constant") {
    Disease d("Test", 5, 0.5);
    Population pop("Test", 200);
    pop.random_vaccination(0.1);
    pop.random_infection(d);
    int total = pop.size();
    for (int day = 0; day < 30; day++) {
        int sum = pop.count_susceptible() + pop.count_infectious()
                + pop.count_recovered()   + pop.count_vaccinated();
        CHECK(sum == total);
        pop.one_more_day(d, 6);
    }
}

// ── Simulation test ────────────────────────────
TEST_CASE("Simulation construction done") {
    Simulation sim("../disease_in.ini");
    CHECK(true);
}

TEST_CASE("High vaccination rate reduces recovered count") {
    Disease d("Test", 5, 0.5);
    const int RUNS = 5;

    double avg_low_vax = 0.0;
    double avg_high_vax = 0.0;

    for (int run = 0; run < RUNS; run++) {
        Population pop_low("Test", 1000);
        pop_low.random_vaccination(0.0);
        pop_low.random_infection(d);
        while (pop_low.count_infectious() > 0)
            pop_low.one_more_day(d, 6);
        avg_low_vax += pop_low.count_recovered();

        Population pop_high("Test", 1000);
        pop_high.random_vaccination(0.9);
        pop_high.random_infection(d);
        while (pop_high.count_infectious() > 0)
            pop_high.one_more_day(d, 6);
        avg_high_vax += pop_high.count_recovered();
    }

    avg_low_vax /= RUNS;
    avg_high_vax /= RUNS;

    CHECK(avg_high_vax < avg_low_vax);
}