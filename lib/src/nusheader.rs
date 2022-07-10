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

    /// Calculates the nus crc sum
    /// similar to libdragon's implementation of chksm64
    pub fn crc(&mut self, data: &[u8]) -> Result<u64, Error> {
        if data.len() < CRC_LEN + CRC_START {
            return Err(Error::NusCrcNotEnoughData);
        }

        let base = 0xffffffff;

        let (mut t0, mut t1, mut t2, mut t3, mut t4, mut t5, mut t6, mut t7, mut t8, mut t9) = (
            0_u32, 0_u32, 0_u32, 0_u32, 0_u32, 0_u32, 0_u32, 0_u32, 0_u32, 0_u32,
        );
        let (mut s0, mut s6) = (0_u32, 0_u32);
        let (mut a0, mut a1, mut a2, mut a3, mut at) = (0_u32, 0_u32, 0_u32, 0_u32, 0_u32);
        let mut lo = 0;
        let (mut v0, mut v1) = (0, 0);
        let mut ra = 0;

        let mut s6 = 0x3f;
        let mut a0 = 0x1000;

        a1 = s6;
        at = 0x5d588b65;

        lo = (a1.wrapping_mul(at)) & base;
        ra = 0x100000;
        v1 = 0;
        t0 = 0;
        t1 = a0;
        t5 = 32;
        v0 = lo;
        v0 = (v0 + 1) & base;
        a3 = v0;
        t2 = v0;
        t3 = v0;
        s0 = v0;
        a2 = v0;
        t4 = v0;

        while t0 != ra {
            // TODO dont unwrap
            v0 = u32::from_be_bytes(data[t1 as usize..t1 as usize + 4].try_into().unwrap());
            v1 = (a3 + v0) & base;
            at = (v1 < a3).into();
            a1 = v1;

            if at != 0 {
                t2 = (t2 + 1) & base;
            }

            v1 = v0 & 0x1F;
            t7 = (t5 - v1) & base;
            t8 = v0 >> t7;
            t6 = (v0 << v1) & base;
            a0 = t6 | t8;
            at = (a2 < v0).into();
            a3 = a1;
            t3 = (t3 ^ v0) & base;
            s0 = (s0 + a0) & base;
            if at != 0 {
                t9 = (a3 ^ v0) & base;

                a2 = (a2 ^ t9) & base;
            } else {
                a2 = (a2 ^ a0) & base;
            }

            t0 += 4;
            t7 = (v0 ^ s0) & base;
            t1 += 4;
            t4 = (t4 + t7) & base;
        }

        t6 = (a3 ^ t2) & base;
        a3 = (t6 ^ t3) & base;
        t8 = (s0 ^ a2) & base;
        s0 = (t8 ^ t4) & base;

        Ok((a3 as u64) << 32 | s0 as u64)
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
