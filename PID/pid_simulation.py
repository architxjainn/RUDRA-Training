import numpy as np
import matplotlib.pyplot as plt

class PIDController:
    def __init__(self, Kp, Ki, Kd, dt):
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.dt = dt
        
        self.integral = 0.0
        self.previous_error = 0.0

    def compute(self, setpoint, measured_value):
        # Calculate error
        error = setpoint - measured_value
        
        # Proportional term
        p_term = self.Kp * error
        
        # Integral term
        self.integral += error * self.dt
        i_term = self.Ki * self.integral
        
        # Derivative term
        derivative = (error - self.previous_error) / self.dt
        d_term = self.Kd * derivative
        
        # Save error for next step
        self.previous_error = error
        
        # Total control output
        return p_term + i_term + d_term

# --- Simulation Parameters ---
dt = 0.01          # Time step (seconds)
total_time = 10.0  # Total duration (seconds)
time_steps = np.arange(0, total_time, dt)

setpoint = 100.0   # Target value (e.g., target motor speed in RPM)

# Simulated System Physical Constants (e.g., motor inertia/friction)
system_value = 0.0  # Initial starting value
system_inertia = 0.15

# Instantiate PID Controller (Try tweaking these values!)
pid = PIDController(Kp=3.5, Ki=2.0, Kd=0.0, dt=dt)

# Arrays to record history for plotting
history_time = []
history_value = []
history_setpoint = []

# --- Run Simulation Loop ---
for t in time_steps:
    # 1. Calculate control output (e.g., motor voltage)
    control_output = pid.compute(setpoint, system_value)
    
    # 2. Update physical system state (Simple 1st-order system dynamics model)
    system_value += (control_output - system_value) * (dt / system_inertia)
    
    # 3. Store history
    history_time.append(t)
    history_value.append(system_value)
    history_setpoint.append(setpoint)

# --- Plotting the Results ---
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.figure(figsize=(10, 5))

plt.plot(history_time, history_setpoint, 'r--', label='Target Setpoint', linewidth=2)
plt.plot(history_time, history_value, 'b-', label='System Response (PID Output)', linewidth=2)

plt.title('PID Controller System Response Simulation', fontsize=14, fontweight='bold')
plt.xlabel('Time (seconds)', fontsize=12)
plt.ylabel('Process Variable (e.g., Speed / Position)', fontsize=12)
plt.axhline(0, color='black', linewidth=0.8, linestyle=':')
plt.legend(loc='lower right', fontsize=11)
plt.tight_layout()

# Display the interactive window on macOS
plt.show()
