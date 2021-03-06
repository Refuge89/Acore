-- MySQL dump 10.13  Distrib 5.6.9-rc, for Win64 (x86_64)
--
-- Host: localhost    Database: unused
-- ------------------------------------------------------
-- Server version	5.6.9-rc

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `movievariation`
--

DROP TABLE IF EXISTS `movievariation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `movievariation` (
  `Id` int(10) NOT NULL DEFAULT '0',
  `Movie` int(10) NOT NULL DEFAULT '0',
  `FileData` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`),
  UNIQUE KEY `Id` (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `movievariation`
--

LOCK TABLES `movievariation` WRITE;
/*!40000 ALTER TABLE `movievariation` DISABLE KEYS */;
/*!40000 ALTER TABLE `movievariation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Records of `movievariation`
--

INSERT INTO `movievariation` VALUES ('3', '2', '2');
INSERT INTO `movievariation` VALUES ('4', '2', '22');
INSERT INTO `movievariation` VALUES ('32', '1', '1');
INSERT INTO `movievariation` VALUES ('33', '1', '21');
INSERT INTO `movievariation` VALUES ('42', '11', '21');
INSERT INTO `movievariation` VALUES ('43', '13', '21');
INSERT INTO `movievariation` VALUES ('61', '14', '220063');
INSERT INTO `movievariation` VALUES ('62', '14', '220062');
INSERT INTO `movievariation` VALUES ('103', '16', '356943');
INSERT INTO `movievariation` VALUES ('104', '16', '356944');
