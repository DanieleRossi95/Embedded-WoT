// WeakPtr.nut

a <- [1, 2, 3, 4, 5];

b <- a.weakref();

print("b[2] = " + b[2] + "\n");

a = null;

print("b[2] = " + b[2] + "\n");
