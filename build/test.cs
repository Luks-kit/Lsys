// Simple ClearSys test file

subr add(int a, int b) returns int {
    return a + b;
}

subr main() returns int {
    int x = 0;
    int y = 5;

    // While loop
    while (x < y) {
        x = x + 1;
    }

    // If statement
    if (x == y) {
        print("x equals y\n");
    } else {
        print("x does not equal y\n");
    }

    int z = add(x, y);
    print("Result: ");
    print(z);
    print("\n");

    return 0;
}

