use std::{array::TryFromSliceError, string::FromUtf8Error};

use thiserror::Error;

#[derive(Debug, Error)]
pub enum Error {
    #[error("Nus rom is too small")]
    CrcNotEnoughData,
    #[error("Header is data cannot be parsed. Input data is too small!")]
    HeaderNotEnoughData,
    #[error("Unable to read")]
    ReadError,
    #[error("Unable to write")]
    WriteError,
    #[error("Unable to convert slice")]
    TryFromSliceError(#[from] TryFromSliceError),
    #[error("Unable to convert utf-8 string")]
    FromUtf8Error(#[from] FromUtf8Error),
}

impl PartialEq for Error {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Self::TryFromSliceError(_l0), Self::TryFromSliceError(_r0)) => false,
            (Self::FromUtf8Error(_), Self::FromUtf8Error(_)) => false,
            _ => core::mem::discriminant(self) == core::mem::discriminant(other),
        }
    }
}
