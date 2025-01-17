from visualization import Simulation

def main():
    # Create and run simulation with 5 processes
    simulation = Simulation(num_processes=20)
    simulation.run_simulation(num_steps=17, snapshot_at_step=7)

if __name__ == "__main__":
    main()