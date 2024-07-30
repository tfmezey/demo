#!/usr/bin/octave -qf

# 6th June 2024 -- 1948 CET
# C++ rk4 vs Assembly rk4.

load thetaplot.txt;
load rplot.txt
load time.txt
load kinetic.txt
load potential.txt

size time
size rplot
size thetaplot

figure
polar(thetaplot, rplot, '+');
xlabel('Distance (Au)');

figure
plot(time, kinetic, '-', time, potential, '--', time, kinetic+potential, '-');
legend('Kinetic', 'Potential', 'Total');
xlabel('Time (yr)');
ylabel('Energy (M AU^2/yr^2)');
