import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("report_data/herd_immunity.csv")
avg = df.groupby("vaccination_rate")["recovered"].mean().reset_index()

plt.figure(figsize=(8, 5))
plt.plot(avg["vaccination_rate"],
            avg["recovered"],
            marker='o',
            linestyle='-',
            linewidth=2,
            markersize=8,
            color="steelblue")
plt.xlabel("Vaccination Rate (%)")
plt.ylabel("Recovered Persons")
plt.title("Recovered vs Vaccination Rate\n(D=5, C=6, β=0.2, R0=6)")
plt.axvline(x=83.3, color="red",
            linestyle="--",
            label="Theoretical HIT = 83.3%")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("report_data/plot1_recovered_vs_vaccination.png", dpi=150)
#plt.show()
print("Plot 1 saved")

details = pd.read_csv("report_data/disease_details.csv")

run0 = details[details["run"] == 0].reset_index(drop=True)
run0["step"] = range(len(run0))

plt.figure(figsize=(8, 5))
plt.plot(run0["step"], run0["infectious"],
         label="infectious",  color="blue")
plt.plot(run0["step"], run0["recovered"],
         label="recovered",   color="green")
plt.plot(run0["step"], run0["susceptible"],
         label="susceptible", color="orange")
plt.plot(run0["step"], run0["vaccinated"],
         label="vaccinated",  color="red")
plt.xlabel("Steps (Days)")
plt.ylabel("Count")
plt.title("Disease Spread Over Time\n(D=5, C=6, β=0.2, vaccination=0%)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("report_data/plot2_timestep.png", dpi=150)
#plt.show()
print("Plot 2 saved")