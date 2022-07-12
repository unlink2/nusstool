use crate::error::Error;

pub trait Header {
    fn from_bytes(data: &[u8]) -> Result<Self, Error>
    where
        Self: Sized;
    fn default_with_crc(&mut self, data: &[u8]) -> Result<Self, Error>
    where
        Self: Sized;

    fn to_bytes(&self) -> Vec<u8>;
    fn set_crc(&mut self, data: &[u8]) -> Result<(), Error>;
}

#[derive(Debug, Default)]
pub struct Buffer {
    pub data: Vec<u8>,
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

    pub fn header<T>(&self) -> Result<T, Error>
    where
        T: Header,
    {
        T::from_bytes(&self.data)
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

    /// inject data into the rom,
    /// if the rom is too small pad until the data can
    /// be placed  
    pub fn inject(&mut self, at: usize, data: &[u8]) {
        // ensure we have enough space in the buffer
        // to actually inject the data
        self.pad_to(at + data.len(), 0x0);

        for (i, val) in data.iter().enumerate() {
            self.data[i] = *val;
        }
    }

    /// inject data from a reader
    pub fn inject_reader(
        &mut self,
        at: usize,
        reader: &mut dyn std::io::Read,
    ) -> Result<(), Error> {
        let mut buffer = vec![];
        reader
            .read_to_end(&mut buffer)
            .map_err(|_e| Error::ReadError)?;
        self.inject(at, &buffer);
        Ok(())
    }
}
