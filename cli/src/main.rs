use clap::{Args, Parser, Subcommand};
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

#[derive(Debug, Args)]
struct NusHeaderInput {
    #[clap(long, value_parser)]
    title: Option<String>,
    #[clap(long, value_parser)]
    boot_addr: Option<u32>,
    #[clap(long, value_parser)]
    cfg_flags: Option<u32>,
    #[clap(long, value_parser)]
    clck_rate: Option<u32>,
    #[clap(long, value_parser)]
    lu_ver: Option<u32>,
    #[clap(long, value_parser)]
    category: Option<char>,
    #[clap(long, value_parser)]
    unique: Option<String>,
    #[clap(long, value_parser)]
    destination: Option<char>,
    #[clap(long, value_parser)]
    version: Option<char>,
}

impl NusHeaderInput {
    /// applies the input data to an existing header
    pub fn apply(&self, header: &mut NusHeader) {
        if let Some(title) = &self.title {
            header.title = title.clone();
        }
        if let Some(boot_addr) = self.boot_addr {
            header.boot_addr = boot_addr;
        }
        if let Some(cfg_flags) = self.cfg_flags {
            header.cfg_flags = cfg_flags;
        }
        if let Some(clck_rate) = self.clck_rate {
            header.clck_rate = clck_rate;
        }
        if let Some(lu_ver) = self.lu_ver {
            header.lu_ver = lu_ver;
        }
        if let Some(category) = self.category {
            header.category = category as u8;
        }
        if let Some(unique) = &self.unique {
            let bytes = unique.as_bytes();
            if let Ok(bytes) = bytes[0..2].try_into() {
                header.unique = bytes;
            }
        }
        if let Some(destination) = self.destination {
            header.destination = destination as u8;
        }
        if let Some(version) = self.version {
            header.version = version as u8;
        }
    }
}

#[derive(Debug, Subcommand)]
enum Commands {
    ShowNusHeader,
    SetNusHeader(NusHeaderInput),
    AddNusHeader(NusHeaderInput),
    CalcNusCrc,
    PadTo { to: usize },
    PadBy { by: usize },
}

fn main() {
    let args = Cli::parse();

    let mut input: Box<dyn std::io::Read> = if let Some(input) = args.input {
        Box::new(File::open(input).expect("Unable to open input file"))
    } else {
        Box::new(std::io::stdin())
    };

    let mut output: Box<dyn std::io::Write> = if let Some(output) = args.output {
        Box::new(
            File::options()
                .write(true)
                .create(true)
                .open(output)
                .expect("Unable to open output file"),
        )
    } else {
        Box::new(std::io::stdout())
    };

    let mut buffer = Buffer::from_reader(&mut input).unwrap();

    match args.command {
        Commands::ShowNusHeader => {
            let header: NusHeader = buffer.header().unwrap();
            println!("{}", header);
        }
        Commands::SetNusHeader(input) => {
            let mut header: NusHeader = buffer.header().unwrap();
            input.apply(&mut header);
            buffer.set_crc(&mut header).unwrap();
            buffer.set_header(&header);
            buffer.write(&mut output).unwrap();
        }
        Commands::AddNusHeader(input) => {
            let mut header = NusHeader::default();
            input.apply(&mut header);
            buffer.set_crc(&mut header).unwrap();
            buffer.add_header(&header);
            buffer.write(&mut output).unwrap();
        }
        Commands::PadTo { to } => {
            buffer.pad_to(to, 0);
            buffer.write(&mut output).unwrap();
        }
        Commands::PadBy { by } => {
            buffer.pad_by(by, 0);
            buffer.write(&mut output).unwrap();
        }
        Commands::CalcNusCrc => {
            let crc = NusHeader::crc(&buffer.data).unwrap();
            println!("{:x}-{:x}", crc.0, crc.1);
        }
    }
}
