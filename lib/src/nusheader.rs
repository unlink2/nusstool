use std::fmt::Display;

use crate::{buffer::Header, error::Error};

pub type Crc = (u32, u32);

#[derive(Debug)]
pub struct NusHeader {
    cfg_flags: u32,
    clck_rate: u32,
    boot_addr: u32,
    lu_ver: u32,
    crc: Crc,
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

impl Display for NusHeader {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(f, "cfg flags: {:x}", self.cfg_flags)?;
        writeln!(f, "clock rate: {:x}", self.clck_rate)?;
        writeln!(f, "lu version: {:x}", self.lu_ver)?;
        writeln!(f, "boot address: {:x}", self.boot_addr)?;
        writeln!(f, "crc: ({:x}-{:x})", self.crc.0, self.crc.1)?;
        writeln!(f, "title: {}", self.title)?;
        writeln!(f, "category: {}", self.category as char)?;
        writeln!(
            f,
            "category: {}{}",
            self.unique[0] as char, self.unique[1] as char
        )?;
        writeln!(f, "destination: {}", self.destination as char)?;
        writeln!(f, "version: {}", self.version)
    }
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

impl Header for NusHeader {
    /// Create a nus header from raw bytes
    fn from_bytes(data: &[u8]) -> Result<Self, Error>
    where
        Self: Sized,
    {
        if data.len() < HEADER_SIZE {
            return Err(Error::HeaderNotEnoughData);
        }

        // TODO this uses a lot of magic offsets
        // but it should be fine because this *literally* will never change (tm)
        Ok(Self {
            cfg_flags: Self::try_u32_from_be(data, 0x00)?,
            clck_rate: Self::try_u32_from_be(data, 0x04)?,
            boot_addr: Self::try_u32_from_be(data, 0x08)?,
            lu_ver: Self::try_u32_from_be(data, 0x0C)?,
            crc: (
                Self::try_u32_from_be(data, 0x10)?,
                Self::try_u32_from_be(data, 0x14)?,
            ),
            reserved_1: Self::try_u64_from_be(data, 0x18)?,
            // 0x14 bytes are reserved for the title
            // it is assumed that it is ascii only!
            title: String::from_utf8(data[0x20..0x34].to_vec())?,
            reserved_2: data[0x34..0x3B].try_into()?,
            category: data[0x3B],
            unique: data[0x3C..0x3E].try_into()?,
            destination: data[0x3E],
            version: data[0x3F],
        })
    }

    /// Converts the NusHeader to
    /// a byte array
    fn to_bytes(&self) -> Vec<u8> {
        let mut header = vec![];

        header.extend(self.cfg_flags.to_be_bytes());
        header.extend(self.clck_rate.to_be_bytes());
        header.extend(self.boot_addr.to_be_bytes());
        header.extend(self.lu_ver.to_be_bytes());
        header.extend(self.crc.0.to_be_bytes());
        header.extend(self.crc.1.to_be_bytes());
        header.extend(self.reserved_1.to_be_bytes());

        // Title always needs to be exactly 0x14 bytes
        let title = format!("{:0<20}", self.title);
        header.extend(&title.as_bytes()[0..TITLE_LEN]);

        header.extend(self.reserved_2);
        header.push(self.category);
        header.extend(self.unique);
        header.push(self.destination);
        header.push(self.version);

        // this panic is justified because
        // it should never occur that the header
        // length is not the correct size, and
        // if it is for some reason it absolutely is a bug
        if header.len() != HEADER_SIZE {
            panic!(
                "Expected vec of length {} but got {}!",
                HEADER_SIZE,
                header.len()
            );
        }

        header
    }

    /// Calculate the nus crc and set the value in the header
    fn set_crc(&mut self, data: &[u8]) -> Result<(), Error> {
        let crc = Self::crc(data)?;
        self.crc = crc;
        Ok(())
    }
}

impl NusHeader {
    fn try_u32_from_be(data: &[u8], at: usize) -> Result<u32, Error> {
        Ok(u32::from_be_bytes(data[at..at + 4].try_into()?))
    }

    fn try_u64_from_be(data: &[u8], at: usize) -> Result<u64, Error> {
        Ok(u64::from_be_bytes(data[at..at + 8].try_into()?))
    }

    /// Calculates the nus crc sum
    /// similar to libdragon's implementation of chksm64
    // TODO clean this up and make it as readable as possible
    pub fn crc(data: &[u8]) -> Result<Crc, Error> {
        if data.len() < CRC_LEN + CRC_START {
            return Err(Error::CrcNotEnoughData);
        }

        let mut t8;
        let mut t6;
        let mut t7;
        let mut t9;

        let mut v1;

        let s6 = 0x3f;
        let mut a0 = 0x1000;

        let mut a1: u32 = s6;
        let mut at = 0x5d588b65;

        let lo = a1.wrapping_mul(at);
        let ra = 0x100000;
        let mut t0 = 0;
        let mut t1 = a0;
        let t5 = 32_u32;
        let mut v0 = lo;
        v0 += 1;
        let mut a3 = v0;
        let mut t2 = v0;
        let mut t3 = v0;
        let mut s0 = v0;
        let mut a2 = v0;
        let mut t4 = v0;

        while t0 != ra {
            v0 = u32::from_be_bytes(data[t1 as usize..t1 as usize + 4].try_into()?);
            v1 = a3.wrapping_add(v0);
            at = (v1 < a3).into();
            a1 = v1;

            if at != 0 {
                t2 = t2.wrapping_add(1);
            }

            v1 = v0 & 0x1F;
            t7 = t5.wrapping_sub(v1);
            t8 = v0 >> t7;
            t6 = v0 << v1;
            a0 = t6 | t8;
            at = (a2 < v0).into();
            a3 = a1;
            t3 ^= v0;
            s0 = s0.wrapping_add(a0);
            if at != 0 {
                t9 = a3 ^ v0;

                a2 ^= t9;
            } else {
                a2 ^= a0;
            }

            t0 += 4;
            t7 = v0 ^ s0;
            t1 += 4;
            t4 = t4.wrapping_add(t7);
        }

        t6 = a3 ^ t2;
        a3 = t6 ^ t3;
        t8 = s0 ^ a2;
        s0 = t8 ^ t4;

        Ok((a3, s0))
    }
}

#[cfg(test)]
mod test {
    use super::*;

    const CRC_END: usize = CRC_START + CRC_LEN;

    #[test]
    fn crc_fail() {
        assert_eq!(Err(Error::CrcNotEnoughData), NusHeader::crc(&[0, 1, 2]));

        // test failure off by 1
        let mut test_data = vec![];
        for i in 0..CRC_END - 1 {
            test_data.push(i as u8);
        }
        assert_eq!(Err(Error::CrcNotEnoughData), NusHeader::crc(&test_data));
    }

    #[test]
    fn crc() {
        let mut test_data = vec![];

        // 0, 1, 2, 3, ... 255, 0, 1, ...
        for i in 0..CRC_END {
            test_data.push(i as u8);
        }

        // result dervice from calculating the checksum
        // using the legacy python script
        assert_eq!(Ok((4207429594, 3000934689)), NusHeader::crc(&test_data));
    }
}
