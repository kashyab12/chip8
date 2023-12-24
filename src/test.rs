fn fib(n: u32) -> u32 {
    return if n <= 2 {
        1
    } else {
        fib(n - 1) + fib(n - 2)
    }
}

pub fn testing_stuff() {
    for i in 1..=5 {
        println!("i: {i}");
    }
}