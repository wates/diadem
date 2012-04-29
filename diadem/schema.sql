-- MySQL dump 10.11
--
-- Host: localhost    Database: ircdb
-- ------------------------------------------------------
-- Server version	5.0.88

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
-- Table structure for table `channel`
--

DROP TABLE IF EXISTS `channel`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `channel` (
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `name` varchar(256) character set utf8 collate utf8_bin NOT NULL,
  `topic` varchar(256) character set utf8 collate utf8_bin default NULL,
  `type` char(1) character set utf8 collate utf8_bin default NULL,
  `mode` varchar(256) character set utf8 collate utf8_bin default NULL,
  PRIMARY KEY  (`name`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `event`
--

DROP TABLE IF EXISTS `event`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `event` (
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `prefix` varchar(256) collate utf8_bin default NULL,
  `command` varchar(32) collate utf8_bin NOT NULL,
  `param1` varchar(512) collate utf8_bin default NULL,
  `param2` varchar(512) collate utf8_bin default NULL,
  `param3` varchar(512) collate utf8_bin default NULL,
  `param4` varchar(512) collate utf8_bin default NULL,
  `param5` varchar(512) collate utf8_bin default NULL,
  `param6` varchar(512) collate utf8_bin default NULL,
  `param7` varchar(512) collate utf8_bin default NULL,
  `param8` varchar(512) collate utf8_bin default NULL,
  `param9` varchar(512) collate utf8_bin default NULL,
  `param10` varchar(512) collate utf8_bin default NULL,
  `param11` varchar(512) collate utf8_bin default NULL,
  `param12` varchar(512) collate utf8_bin default NULL,
  KEY `time` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `join_nick`
--

DROP TABLE IF EXISTS `join_nick`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `join_nick` (
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `nick_name` varchar(256) collate utf8_bin NOT NULL,
  `channel_name` varchar(256) collate utf8_bin NOT NULL,
  `is_operator` tinyint(1) NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `message`
--

DROP TABLE IF EXISTS `message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `message` (
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `nick` varchar(256) collate utf8_bin NOT NULL,
  `target` varchar(256) collate utf8_bin NOT NULL,
  `text` text collate utf8_bin NOT NULL,
  `is_notice` int(11) NOT NULL,
  KEY `time` (`time`),
  KEY `time_2` (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `motd`
--

DROP TABLE IF EXISTS `motd`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `motd` (
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `server` varchar(256) collate utf8_bin NOT NULL,
  `message` text collate utf8_bin NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `query_error`
--

DROP TABLE IF EXISTS `query_error`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `query_error` (
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `query` text collate utf8_bin NOT NULL,
  `message` text collate utf8_bin NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `queue`
--

DROP TABLE IF EXISTS `queue`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `queue` (
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `command` varchar(256) collate utf8_bin NOT NULL,
  `parameter` varchar(256) collate utf8_bin default NULL,
  `message` text collate utf8_bin
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `user`
--

DROP TABLE IF EXISTS `user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user` (
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `nick` varchar(256) collate utf8_bin NOT NULL,
  `realname` varchar(256) collate utf8_bin NOT NULL,
  `hostname` varchar(256) collate utf8_bin NOT NULL,
  PRIMARY KEY  (`nick`),
  KEY `nick` (`nick`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2012-01-06 10:42:25
