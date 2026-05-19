s = serialport("COM15", 1500000); % Adjust baud rate as per your setup

while true
    data = readline(s); % Read a line of data from the serial port

    if startsWith(data, "A5A5") % If data starts with "A5A5"
        ch1 = typecast(uint8(data(5:6)), 'uint16'); % Extract first 16-bit data
        ch2 = typecast(uint8(data(7:8)), 'uint16'); % Extract second 16-bit data
        ch3 = typecast(uint8(data(9:10)), 'uint16'); % Extract third 16-bit data

        disp("Channel 1 Data: " + ch1); 
        disp("Channel 2 Data: " + ch2); 
        disp("Channel 3 Data: " + ch3); 
    end
end

clear s % Remove serial port object from workspace
