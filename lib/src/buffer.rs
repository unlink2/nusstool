use crate::error::Error;

pub trait Header {
    fn to_bytes(&self) -> Vec<u8>;
    fn set_crc(&mut self, data: &[u8]) -> Result<(), Error>;
}

#[derive(Default)]
pub struct Buffer {
    data: Vec<u8>,
}

impl Buffer {
    pub fn new() -> Self {
        Default::default()
    }

    pub fn from_reader(reader: &mut dyn std::io::Read) -> Result<Self, Error> {
        let mut s = Self::default();

        reader
            .read_to_end(&mut s.data)
            .map_err(|_e| Error::ReadError)?;

        Ok(s)
    }

    pub fn write(&self, writer: &mut dyn std::io::Write) -> Result<(), Error> {
        writer.write_all(&self.data).map_err(|_e| Error::WriteError)
    }

    pub fn set_crc(&self, header: &mut dyn Header) -> Result<(), Error> {
        header.set_crc(&self.data)
    }

    /// Append a header to the current buffer
    pub fn add_header(&mut self, header: &dyn Header) {
        let mut header_bytes = header.to_bytes();
        header_bytes.append(&mut self.data);
        self.data = header_bytes;
    }

    /// Overwrite the first HEADER_SIZE bytes in the buffer with a new header
    pub fn set_header(&mut self, header: &dyn Header) {
        let header_bytes = header.to_bytes();
        for (i, val) in header_bytes.iter().enumerate() {
            self.data[i] = *val;
        }
    }

    pub fn pad_to(&mut self, to: usize, val: u8) {
        while self.data.len() != to {
            self.data.push(val);
        }
    }

    pub fn pad_by(&mut self, by: usize, val: u8) {
        for _i in 0..by {
            self.data.push(val);
        }
    }
}
