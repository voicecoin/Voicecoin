-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Server Version:               10.1.21-MariaDB - MariaDB Server
-- Server Betriebssystem:        Linux
-- HeidiSQL Version:             9.4.0.5151
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Exportiere Struktur von Tabelle emcchain.address
CREATE TABLE IF NOT EXISTS `address` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `address` varchar(50) DEFAULT NULL,
  `balance` double DEFAULT NULL,
  `account` varchar(50) DEFAULT NULL,
  `last_sent` int(10) unsigned DEFAULT NULL,
  `last_received` int(10) unsigned DEFAULT NULL,
  `count_sent` int(10) unsigned DEFAULT NULL,
  `count_received` int(10) unsigned DEFAULT NULL,
  `total_sent` double DEFAULT NULL,
  `total_received` double DEFAULT NULL,
  `status` char(1) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=210167 DEFAULT CHARSET=latin1 ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=8;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle emcchain.blocks
CREATE TABLE IF NOT EXISTS `blocks` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `hash` varchar(65) NOT NULL,
  `size` int(10) unsigned DEFAULT NULL,
  `height` int(10) unsigned DEFAULT NULL,
  `version` int(10) unsigned DEFAULT NULL,
  `merkleroot` varchar(65) DEFAULT NULL,
  `time` int(10) unsigned DEFAULT NULL,
  `nonce` varchar(50) DEFAULT NULL,
  `bits` varchar(50) DEFAULT NULL,
  `difficulty` varchar(50) DEFAULT NULL,
  `mint` double DEFAULT NULL,
  `previousblockhash` varchar(65) DEFAULT NULL,
  `flags` varchar(50) DEFAULT NULL,
  `proofhash` varchar(65) DEFAULT NULL,
  `entropybit` bit(1) DEFAULT NULL,
  `modifier` varchar(50) DEFAULT NULL,
  `modifierchecksum` varchar(50) DEFAULT NULL,
  `numtx` int(10) unsigned DEFAULT NULL,
  `numvin` int(10) unsigned DEFAULT NULL,
  `numvout` int(10) unsigned DEFAULT NULL,
  `valuein` double DEFAULT NULL,
  `valueout` double DEFAULT NULL,
  `fee` double DEFAULT NULL,
  `total_coins` double DEFAULT NULL,
  `coindaysdestroyed` double DEFAULT NULL,
  `avgcoindaysdestroyed` double DEFAULT NULL,
  `total_coindays` double DEFAULT NULL,
  `total_avgcoindays` double DEFAULT NULL,
  `total_addresses_used` int(10) unsigned DEFAULT NULL,
  `total_addresses_unused` int(10) unsigned DEFAULT NULL,
  `status` char(1) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=215727 DEFAULT CHARSET=latin1 ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=8;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle emcchain.nvs
CREATE TABLE IF NOT EXISTS `nvs` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(1000) DEFAULT NULL,
  `value` varchar(25000) DEFAULT NULL,
  `type` varchar(50) DEFAULT NULL,
  `isbase64` char(1) DEFAULT NULL,
  `registered_at` int(10) unsigned DEFAULT NULL,
  `expires_at` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=47339 DEFAULT CHARSET=latin1 ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=8;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle emcchain.transactions
CREATE TABLE IF NOT EXISTS `transactions` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `blockid` int(10) unsigned NOT NULL,
  `txid` varchar(65) NOT NULL,
  `time` int(10) unsigned DEFAULT NULL,
  `numvin` int(10) unsigned DEFAULT NULL,
  `numvout` int(10) unsigned DEFAULT NULL,
  `valuein` double DEFAULT NULL,
  `valueout` double DEFAULT NULL,
  `fee` double DEFAULT NULL,
  `coindaysdestroyed` double DEFAULT NULL,
  `avgcoindaysdestroyed` double DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=539793 DEFAULT CHARSET=latin1 ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=8;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle emcchain.vin
CREATE TABLE IF NOT EXISTS `vin` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `blockid` int(10) unsigned NOT NULL,
  `parenttxid` int(10) unsigned NOT NULL,
  `output_txid` varchar(65) NOT NULL,
  `coinbase` varchar(65) DEFAULT NULL,
  `vout` int(10) unsigned DEFAULT NULL,
  `asm` varchar(1000) DEFAULT NULL,
  `hex` varchar(1000) DEFAULT NULL,
  `sequence` varchar(50) DEFAULT NULL,
  `address` varchar(50) DEFAULT NULL,
  `value` double DEFAULT NULL,
  `coindaysdestroyed` double DEFAULT NULL,
  `avgcoindaysdestroyed` double DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=740913 DEFAULT CHARSET=latin1 ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=8;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle emcchain.vout
CREATE TABLE IF NOT EXISTS `vout` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `blockid` int(10) unsigned NOT NULL,
  `parenttxid` int(10) unsigned NOT NULL,
  `value` double DEFAULT NULL,
  `n` int(10) unsigned DEFAULT NULL,
  `asm` varchar(1000) DEFAULT NULL,
  `hex` varchar(1000) DEFAULT NULL,
  `reqsigs` int(10) unsigned DEFAULT NULL,
  `type` varchar(50) DEFAULT NULL,
  `address` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=955265 DEFAULT CHARSET=latin1 ROW_FORMAT=COMPRESSED KEY_BLOCK_SIZE=8;

-- Daten Export vom Benutzer nicht ausgewählt
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
