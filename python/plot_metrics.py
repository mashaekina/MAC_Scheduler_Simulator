import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def main():
    parser = argparse.ArgumentParser(description="Plot scheduler metrics from CSV")
    parser.add_argument("csv", type=Path, help="Path to metrics CSV file")
    parser.add_argument("--output", type=Path, default=Path("metrics.png"))
    args = parser.parse_args()

    xs = []
    throughput = []
    fairness = []

    with args.csv.open() as f:
        reader = csv.DictReader(f)
        for row in reader:
            xs.append(int(row.get("tti", 0)))
            throughput.append(float(row.get("throughput_rb", 0)))
            fairness.append(float(row.get("fairness", 0)))

    fig, ax1 = plt.subplots()
    ax1.plot(xs, throughput, label="Throughput", color="tab:blue")
    ax1.set_xlabel("TTI")
    ax1.set_ylabel("Throughput (bits)", color="tab:blue")
    ax1.tick_params(axis="y", labelcolor="tab:blue")

    ax2 = ax1.twinx()
    ax2.plot(xs, fairness, label="Fairness", color="tab:orange")
    ax2.set_ylabel("Jain Fairness", color="tab:orange")
    ax2.tick_params(axis="y", labelcolor="tab:orange")

    fig.tight_layout()
    fig.savefig(args.output)
    print(f"Wrote plot to {args.output}")


if __name__ == "__main__":
    main()
