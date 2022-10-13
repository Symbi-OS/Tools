use std::arch::asm;

#[link(name = "libelevate")]
extern "C" {
    fn sym_elevate();
}
#[link(name = "libelevate")]
extern "C" {
    fn sym_lower() ;
}

fn get_reg(){
    let mut x: u64;

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
// asm_foo();
    get_reg(42);
}
