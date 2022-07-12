use clap::{Parser, Subcommand};
use nusstool::{buffer::Buffer, nusheader::NusHeader};
use std::fs::File;

#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]
struct Cli {
    #[clap(subcommand)]
    command: Commands,

    #[clap(short, long, value_parser)]
    input: Option<String>,
    #[clap(short, long, value_parser)]
    output: Option<String>,
}

#[derive(Debug, Subcommand)]
enum Commands {
    ShowNusHeader,
    SetNusHeader,
    AddNusHeader,
    CalcNusCrc,
    PadTo,
    PadBy,
}

fn main() {
    let args = Cli::parse();

    let mut input: Box<dyn std::io::Read> = if let Some(input) = args.input {
        Box::new(File::open(input).expect("Unable to open input file"))
    } else {
        Box::new(std::io::stdin())
    };

    let _output: Box<dyn std::io::Write> = if let Some(output) = args.output {
        Box::new(File::open(output).expect("Unable to open output file"))
    } else {
        Box::new(std::io::stdout())
    };

    match args.command {
        Commands::ShowNusHeader => {
            let buffer = Buffer::from_reader(&mut input).unwrap();
            let header: NusHeader = buffer.header().unwrap();
            println!("{}", header);
        }
        Commands::SetNusHeader => {}
        Commands::AddNusHeader => {}
        Commands::PadTo => {}
        Commands::PadBy => {}
        Commands::CalcNusCrc => {
            let mut buffer = vec![];
            input.read_to_end(&mut buffer).unwrap();

            let crc = NusHeader::crc(&buffer).unwrap();
            println!("{:x}-{:x}", crc.0, crc.1);
        }
    }
}
