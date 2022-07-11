use clap::Parser;

#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]
pub struct Commands {}

fn main() {
    println!("nustool!");
}

