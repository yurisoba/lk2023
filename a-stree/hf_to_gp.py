# convert hyperfine output json to gnuplot

"""
command: hyperfine --warmup 20 -L n 100000,200000,300000,400000,500000,600000,700000,800000,900000,1000000,2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000,10000000 --export-markdown out.md --export-json out.json './rbtest {n} 1337' './stree {n} 1337'
compile flags: -O2
"""

import json
import os

with open(os.path.join("results", "out.json")) as f:
    results = json.load(f)["results"]

items = {}
for result in results:
    cmd = result["command"].split(" ")[0].replace("./", "")
    if cmd not in items:
        items[cmd] = []
    items[cmd].append({
        "n": int(result["parameters"]["n"]),
        "mean": float(result["mean"]),
        "min": float(result["min"]),
        "max": float(result["max"])
    })

for cmd in items:
    with open(os.path.join("results", cmd + ".dat"), "w") as f:
        f.write("# " + cmd + "\n")
        f.write("#\tn\tmean\tmin\tmax\n")
        for row in items[cmd]:
            f.write(f"\t{row['n']}\t{row['mean']}\t{row['min']}\t{row['max']}\n")

