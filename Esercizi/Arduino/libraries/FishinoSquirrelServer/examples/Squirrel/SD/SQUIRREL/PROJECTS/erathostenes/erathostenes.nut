// erathostenes.nut
// get all prime numbers from 1 to n

function erathostenes(n) {
    // Eratosthenes algorithm to find all primes under n
    local arr = [], upperLimit = Math.sqrt(n), output = [2];

    // Make an array from 2 to (n - 1)
    for (local i = 0; i < n; i++)
        arr.append(1);

    // Remove multiples of primes starting from 2, 3, 5,...
    for (local i = 3; i <= upperLimit; i += 2) {
        if (arr[i]) {
            for (local j = i * i; j < n; j += i*2)
                arr[j] = 0;
        }
    }

    // All array[i] set to 1 (true) are primes
    for (local i = 3; i < n; i += 2) {
        if(arr[i]) {
            output.append(i);
        }
    }
	arr.clear();

    foreach(i in output)
		print(i + "\n");
	output.clear();
};

erathostenes(1000);
