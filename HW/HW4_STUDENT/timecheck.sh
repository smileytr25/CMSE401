#!/bin/bash
#SBATCH --job-name=revGOL_batch
#SBATCH --output=revGOL_output.txt
#SBATCH --error=revGOL_error.txt
#SBATCH --time=04:00:00           # HH:MM:SS
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G
set -e

total_time=0
best_fitness=""
best_seed=""

echo "Running program 10 times with cmse2.txt..."

for seed in {1..10}; do
    start=$(date +%s.%N)
    ./revGOL cmse2.txt
    end=$(date +%s.%N)

    runtime=$(echo "$end - $start" | bc)
    total_time=$(echo "$total_time + $runtime" | bc)
done

avg_time=$(echo "$total_time / 10" | bc -l)

echo ""
echo "✅ All runs complete."
echo "Average Runtime: $avg_time seconds"
