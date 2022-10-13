use std::fs::File;
use std::io::Write;

use std::arch::asm;

#[link(name = "libelevate")]
extern "C" {
    fn sym_elevate();
}
#[link(name = "libelevate")]
extern "C" {
    fn sym_lower() ;
}

fn get_reg(_my_in: u64){
    let mut x: u64 = 4;


    unsafe {
        sym_elevate();
        asm!(
            "mov {x}, cr3",
            x = inout(reg) x,
        );
        sym_lower();
    }

    println!("rdi was {}", x);

}

fn asm_foo(){
// Multiply x by 6 using shifts and adds
    let mut x: u64 = 4;
unsafe {
    asm!(
        "mov {tmp}, {x}",
        "shl {tmp}, 1",
        "shl {x}, 2",
        "add {x}, {tmp}",
        x = inout(reg) x,
        tmp = out(reg) _,
    );
}
assert_eq!(x, 4 * 6);
}

fn main() {

    asm_foo();

    get_reg(42);

    // Create a temporary file.
    // This creates the file if it does not exist (and empty the file if it exists).
    let mut file = File::create("./file").unwrap();



    // Write a byte string.
    let mut i = 0;
    while i < 1000000 {
        i = i + 1;
        file.write(b"Bytes\n").unwrap();
    }
}
