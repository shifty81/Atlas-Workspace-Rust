use serde::{Deserialize, Serialize};
use std::io::Write;

pub const SAVE_MAGIC: u32 = 0x41534156;
pub const PARTIAL_SAVE_MAGIC: u32 = 0x41535057;

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct SaveHeader {
    pub magic: u32,
    pub version: u32,
    pub tick_rate: u32,
    pub save_tick: u64,
    pub state_hash: u64,
    pub seed: u64,
    pub ecs_data_size: u32,
    pub aux_data_size: u32,
    pub metadata_size: u32,
}

impl SaveHeader {
    const SIZE: usize = 4 + 4 + 4 + 8 + 8 + 8 + 4 + 4 + 4; // 48 bytes

    fn write<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        w.write_all(&self.magic.to_le_bytes())?;
        w.write_all(&self.version.to_le_bytes())?;
        w.write_all(&self.tick_rate.to_le_bytes())?;
        w.write_all(&self.save_tick.to_le_bytes())?;
        w.write_all(&self.state_hash.to_le_bytes())?;
        w.write_all(&self.seed.to_le_bytes())?;
        w.write_all(&self.ecs_data_size.to_le_bytes())?;
        w.write_all(&self.aux_data_size.to_le_bytes())?;
        w.write_all(&self.metadata_size.to_le_bytes())?;
        Ok(())
    }

    fn read(data: &[u8]) -> Option<Self> {
        if data.len() < Self::SIZE {
            return None;
        }
        let mut off = 0;
        macro_rules! rd32 {
            () => {{
                let v = u32::from_le_bytes(data[off..off + 4].try_into().ok()?);
                off += 4;
                v
            }};
        }
        macro_rules! rd64 {
            () => {{
                let v = u64::from_le_bytes(data[off..off + 8].try_into().ok()?);
                off += 8;
                v
            }};
        }
        Some(SaveHeader {
            magic: rd32!(),
            version: rd32!(),
            tick_rate: rd32!(),
            save_tick: rd64!(),
            state_hash: rd64!(),
            seed: rd64!(),
            ecs_data_size: rd32!(),
            aux_data_size: rd32!(),
            metadata_size: rd32!(),
        })
    }
}

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct PartialSaveHeader {
    pub magic: u32,
    pub version: u32,
    pub tick_rate: u32,
    pub save_tick: u64,
    pub state_hash: u64,
    pub seed: u64,
    pub chunk_count: u32,
}

impl PartialSaveHeader {
    const SIZE: usize = 4 + 4 + 4 + 8 + 8 + 8 + 4; // 40 bytes

    fn write<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        w.write_all(&self.magic.to_le_bytes())?;
        w.write_all(&self.version.to_le_bytes())?;
        w.write_all(&self.tick_rate.to_le_bytes())?;
        w.write_all(&self.save_tick.to_le_bytes())?;
        w.write_all(&self.state_hash.to_le_bytes())?;
        w.write_all(&self.seed.to_le_bytes())?;
        w.write_all(&self.chunk_count.to_le_bytes())?;
        Ok(())
    }

    fn read(data: &[u8]) -> Option<Self> {
        if data.len() < Self::SIZE {
            return None;
        }
        let mut off = 0;
        macro_rules! rd32 {
            () => {{
                let v = u32::from_le_bytes(data[off..off + 4].try_into().ok()?);
                off += 4;
                v
            }};
        }
        macro_rules! rd64 {
            () => {{
                let v = u64::from_le_bytes(data[off..off + 8].try_into().ok()?);
                off += 8;
                v
            }};
        }
        Some(PartialSaveHeader {
            magic: rd32!(),
            version: rd32!(),
            tick_rate: rd32!(),
            save_tick: rd64!(),
            state_hash: rd64!(),
            seed: rd64!(),
            chunk_count: rd32!(),
        })
    }
}

#[derive(Debug, Clone, Default)]
pub struct ChunkSaveEntry {
    pub x: i32,
    pub y: i32,
    pub z: i32,
    pub data: Vec<u8>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum SaveResult {
    Success,
    FileNotFound,
    InvalidFormat,
    VersionMismatch,
    HashMismatch,
    IoError,
}

const CURRENT_VERSION: u32 = 1;

#[derive(Default)]
pub struct SaveSystem {
    header: SaveHeader,
    ecs_data: Vec<u8>,
    aux_data: Vec<u8>,
    metadata: String,
    partial_header: PartialSaveHeader,
    chunks: Vec<ChunkSaveEntry>,
}

impl SaveSystem {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn save(
        &mut self,
        path: &str,
        tick: u64,
        tick_rate: u32,
        seed: u64,
        ecs_data: &[u8],
        aux_data: &[u8],
        metadata: &str,
    ) -> SaveResult {
        let meta_bytes = metadata.as_bytes();
        let header = SaveHeader {
            magic: SAVE_MAGIC,
            version: CURRENT_VERSION,
            tick_rate,
            save_tick: tick,
            state_hash: 0,
            seed,
            ecs_data_size: ecs_data.len() as u32,
            aux_data_size: aux_data.len() as u32,
            metadata_size: meta_bytes.len() as u32,
        };
        let mut buf = Vec::new();
        if header.write(&mut buf).is_err() {
            return SaveResult::IoError;
        }
        buf.extend_from_slice(ecs_data);
        buf.extend_from_slice(aux_data);
        buf.extend_from_slice(meta_bytes);
        match std::fs::write(path, &buf) {
            Ok(_) => SaveResult::Success,
            Err(_) => SaveResult::IoError,
        }
    }

    pub fn load(&mut self, path: &str) -> SaveResult {
        let data = match std::fs::read(path) {
            Ok(d) => d,
            Err(_) => return SaveResult::FileNotFound,
        };
        let hdr = match SaveHeader::read(&data) {
            Some(h) => h,
            None => return SaveResult::InvalidFormat,
        };
        if hdr.magic != SAVE_MAGIC {
            return SaveResult::InvalidFormat;
        }
        if hdr.version != CURRENT_VERSION {
            return SaveResult::VersionMismatch;
        }
        let base = SaveHeader::SIZE;
        let ecs_end = base + hdr.ecs_data_size as usize;
        let aux_end = ecs_end + hdr.aux_data_size as usize;
        let meta_end = aux_end + hdr.metadata_size as usize;
        if data.len() < meta_end {
            return SaveResult::InvalidFormat;
        }
        self.ecs_data = data[base..ecs_end].to_vec();
        self.aux_data = data[ecs_end..aux_end].to_vec();
        self.metadata = String::from_utf8_lossy(&data[aux_end..meta_end]).into_owned();
        self.header = hdr;
        SaveResult::Success
    }

    pub fn validate(&self, path: &str) -> SaveResult {
        let data = match std::fs::read(path) {
            Ok(d) => d,
            Err(_) => return SaveResult::FileNotFound,
        };
        let hdr = match SaveHeader::read(&data) {
            Some(h) => h,
            None => return SaveResult::InvalidFormat,
        };
        if hdr.magic != SAVE_MAGIC {
            return SaveResult::InvalidFormat;
        }
        if hdr.version != CURRENT_VERSION {
            return SaveResult::VersionMismatch;
        }
        SaveResult::Success
    }

    pub fn header(&self) -> &SaveHeader {
        &self.header
    }

    pub fn ecs_data(&self) -> &[u8] {
        &self.ecs_data
    }

    pub fn aux_data(&self) -> &[u8] {
        &self.aux_data
    }

    pub fn metadata(&self) -> &str {
        &self.metadata
    }

    pub fn clear(&mut self) {
        *self = Self::default();
    }

    pub fn save_partial(
        &mut self,
        path: &str,
        tick: u64,
        tick_rate: u32,
        seed: u64,
        chunks: &[ChunkSaveEntry],
    ) -> SaveResult {
        let hdr = PartialSaveHeader {
            magic: PARTIAL_SAVE_MAGIC,
            version: CURRENT_VERSION,
            tick_rate,
            save_tick: tick,
            state_hash: 0,
            seed,
            chunk_count: chunks.len() as u32,
        };
        let mut buf = Vec::new();
        if hdr.write(&mut buf).is_err() {
            return SaveResult::IoError;
        }
        for chunk in chunks {
            buf.extend_from_slice(&chunk.x.to_le_bytes());
            buf.extend_from_slice(&chunk.y.to_le_bytes());
            buf.extend_from_slice(&chunk.z.to_le_bytes());
            buf.extend_from_slice(&(chunk.data.len() as u32).to_le_bytes());
            buf.extend_from_slice(&chunk.data);
        }
        match std::fs::write(path, &buf) {
            Ok(_) => SaveResult::Success,
            Err(_) => SaveResult::IoError,
        }
    }

    pub fn load_partial(&mut self, path: &str) -> SaveResult {
        let data = match std::fs::read(path) {
            Ok(d) => d,
            Err(_) => return SaveResult::FileNotFound,
        };
        let hdr = match PartialSaveHeader::read(&data) {
            Some(h) => h,
            None => return SaveResult::InvalidFormat,
        };
        if hdr.magic != PARTIAL_SAVE_MAGIC {
            return SaveResult::InvalidFormat;
        }
        if hdr.version != CURRENT_VERSION {
            return SaveResult::VersionMismatch;
        }
        let mut off = PartialSaveHeader::SIZE;
        let mut chunks = Vec::new();
        for _ in 0..hdr.chunk_count {
            if off + 16 > data.len() {
                return SaveResult::InvalidFormat;
            }
            let x = i32::from_le_bytes(data[off..off + 4].try_into().unwrap());
            off += 4;
            let y = i32::from_le_bytes(data[off..off + 4].try_into().unwrap());
            off += 4;
            let z = i32::from_le_bytes(data[off..off + 4].try_into().unwrap());
            off += 4;
            let len = u32::from_le_bytes(data[off..off + 4].try_into().unwrap()) as usize;
            off += 4;
            if off + len > data.len() {
                return SaveResult::InvalidFormat;
            }
            let chunk_data = data[off..off + len].to_vec();
            off += len;
            chunks.push(ChunkSaveEntry { x, y, z, data: chunk_data });
        }
        self.partial_header = hdr;
        self.chunks = chunks;
        SaveResult::Success
    }

    pub fn partial_header(&self) -> &PartialSaveHeader {
        &self.partial_header
    }

    pub fn chunks(&self) -> &[ChunkSaveEntry] {
        &self.chunks
    }
}
