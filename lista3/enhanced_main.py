from visualization import EnhancedSimulation

def main():
    # Create and run simulation with 5 processes
    simulation = EnhancedSimulation(num_processes=5)
    simulation.run_simulation(num_steps=15, snapshot_at_step=7)

if __name__ == "__main__":
    main()