use std::ascii::AsciiExt;

use crate::error::Error;

pub struct NusHeader {
    cfg_flags: u32,
    clck_rate: u32,
    boot_addr: u32,
    lu_ver: u32,
    crc: u64,
    reserved_1: u64,
    // will be cut off after 0x14 bytes,
    // therefore spilling into the
    // reserved part after
    title: String,
    reserved_2: [u8; 0x07],
    category: u8,
    unique: [u8; 2],
    destination: u8,
    version: u8,
}

impl Default for NusHeader {
    fn default() -> Self {
        Self {
            cfg_flags: 0x80_37_12_40,
            clck_rate: 0xF,
            boot_addr: 0x80010000,
            lu_ver: Default::default(),
            crc: Default::default(),
            reserved_1: Default::default(),
            title: Default::default(),
            reserved_2: Default::default(),
            category: b'N',
            unique: Default::default(),
            destination: b'A',
            version: Default::default(),
        }
    }
}

const HEADER_SIZE: usize = 0x40;
const CRC_START: usize = 0x1000;
const CRC_LEN: usize = 0x100000;
const TITLE_LEN: usize = 0x14;
const CRC_START_VAL: u32 = 0xf8ca4ddc;

impl NusHeader {
    /// Converts the NusHeader to
    /// a byte array
    pub fn to_bytes(&self) -> [u8; HEADER_SIZE] {
        let mut header = vec![];

        header.extend(self.cfg_flags.to_be_bytes());
        header.extend(self.clck_rate.to_be_bytes());
        header.extend(self.boot_addr.to_be_bytes());
        header.extend(self.lu_ver.to_be_bytes());
        header.extend(self.crc.to_be_bytes());
        header.extend(self.reserved_1.to_be_bytes());

        // Title always needs to be exactly 0x14 bytes
        let title = format!("{:0<20}", self.title);
        header.extend(&title.as_bytes()[0..TITLE_LEN]);

        header.extend(self.reserved_2);
        header.push(self.category);
        header.extend(self.unique);
        header.push(self.destination);
        header.push(self.version);

        header.try_into().unwrap_or_else(|v: Vec<u8>| {
            panic!(
                "Expected vec of length {} but got {}!",
                HEADER_SIZE,
                v.len()
            )
        })
    }

    /// Calculates the nus crc sum based on Nagra's program
    /// similar to libdragon's implementation of chksm64
    pub fn crc(&mut self, data: &[u8]) -> Result<u64, Error> {
        if data.len() < CRC_LEN + CRC_START {
            return Err(Error::NusCrcNotEnoughData);
        }

        let (crchi, crclo) = (0, 0); // a3 and s0 result

        let s6 = 0x3F;
        let a0 = CRC_START;
        let a1 = s6;
        let a3: u32 = 0;

        // idnex is t0 and t1
        for index in (0..CRC_LEN).step_by(4) {
            // TODO dont unwrap!
            // v0
            let val = u32::from_be_bytes(data[index..index + 4].try_into().unwrap());

            let v1 = a3.wrapping_add(val);
        }

        Ok(crchi << 32 | crclo)
    }
}

#[cfg(test)]
mod test {
    use super::NusHeader;

    #[test]
    fn convert_to_bytes() {
        let h = NusHeader::default();

        h.to_bytes();
    }
}
