use thiserror::Error;

#[derive(Debug, Eq, PartialEq, Error)]
pub enum Error {
    #[error("Nus rom is too small")]
    NusCrcNotEnoughData,
    #[error("Unable to read")]
    ReadError,
    #[error("Unable to write")]
    WriteError,
}
