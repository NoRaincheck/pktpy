def is_prime(x):
    if x < 2:
        return False
    for i in range(2, x):
        if x % i == 0:
            return False
    return True


primes = [i for i in range(2, 20) if is_prime(i)]
print(primes)
# [2, 3, 5, 7, 11, 13, 17, 19]
