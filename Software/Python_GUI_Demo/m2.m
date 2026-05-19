s = serialport("COM15", 1500000); % Adjust baud rate as per your setup

% Preallocation for speed
dataLength = 1000;
ch1_data = zeros(1, dataLength);
ch2_data = zeros(1, dataLength);
ch3_data = zeros(1, dataLength);

% Create figure and three axes for plotting
figure;
ax1 = subplot(3,1,1); ylabel('Channel 1');
ax2 = subplot(3,1,2); ylabel('Channel 2');
ax3 = subplot(3,1,3); ylabel('Channel 3');
xlabel('Sample Number');

counter = 1;
while counter <= dataLength
    data = readline(s); % Read a line of data from the serial port

    if startsWith(data, "A5A5") % If data starts with "A5A5"
        ch1_data(counter) = typecast(uint8(data(5:6)), 'uint16'); % Extract first 16-bit data
        ch2_data(counter) = typecast(uint8(data(7:8)), 'uint16'); % Extract second 16-bit data
        ch3_data(counter) = typecast(uint8(data(9:10)), 'uint16'); % Extract third 16-bit data

        % Refresh plots
        plot(ax1, ch1_data); 
        plot(ax2, ch2_data); 
        plot(ax3, ch3_data); 
        drawnow;
        
        counter = counter + 1;
    end
end

clear s % Remove serial port object from workspace
