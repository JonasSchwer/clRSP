function out = roundUp(value, multiple)
remainder = mod(value,multiple);
if remainder ~= 0
    value = value + multiple - remainder;
end
out = value;