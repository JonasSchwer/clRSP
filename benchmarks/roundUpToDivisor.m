function out = roundUpToDivisor(value, numerator)
out = value;
rem = mod(numerator, out);
while rem ~= 0
    out = out + 1;
    rem = mod(numerator, out);
end