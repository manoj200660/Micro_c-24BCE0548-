func main_prog() {
    int x;
    int y;
    int result;
    int i;
    int arr[5];

    x = 10;
    y = 3;

    result = x + y * 2;
    arr[0] = result;
    arr[1] = x - y;

    if (x > y) {
        result = arr[0] + arr[1];
    } else {
        result = x * y;
    }

    for (i = 0; i < 5; i = i + 1) {
        result = result + i;
    }

    print result;
    return result;
}