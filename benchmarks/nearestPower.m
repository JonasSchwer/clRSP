function [out, exps] = nearestPower(n, primes)

tmp = n;
[out, exps] = isPower(tmp, primes);
while out ~= tmp
    tmp = tmp + 1;
    [out, exps] = isPower(tmp, primes);
end

function [out, exponents] = isPower(n, primes)

rest = n;
exponents = zeros(1,length(primes));

for i = 1:length(primes)
    while mod(rest,primes(i)) == 0
        rest = rest/primes(i);
        exponents(i) = exponents(i) + 1;
    end
end

out = prod(primes.^exponents);