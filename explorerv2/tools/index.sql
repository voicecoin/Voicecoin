  CREATE INDEX idx_blocks_height ON blocks (height);
 CREATE INDEX idx_blocks_hash ON blocks (hash);
 
 
 CREATE INDEX idx_transactions_blockid ON transactions (blockid); 
