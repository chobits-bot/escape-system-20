-- ============================================
-- 建筑图节点表：房间、通道交叉点、出口、楼梯等
-- ============================================
CREATE TABLE IF NOT EXISTS `graph_node` (
  `id`         INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `floor_id`   INT UNSIGNED NOT NULL COMMENT '所属楼层(1=1F, 2=2F...)',
  `node_key`   VARCHAR(64)  NOT NULL COMMENT '节点唯一标识(如 f1-lobby, stair-east)',
  `name`       VARCHAR(128) NOT NULL COMMENT '节点名称',
  `node_type`  ENUM('room','corridor','exit','stair','elevator') NOT NULL,
  `x`          DECIMAL(8,2) NOT NULL COMMENT 'X坐标(米)',
  `y`          DECIMAL(8,2) NOT NULL DEFAULT 0 COMMENT 'Y坐标/高度(米)',
  `z`          DECIMAL(8,2) NOT NULL COMMENT 'Z坐标(米)',
  `capacity`   INT UNSIGNED DEFAULT NULL COMMENT '最大容纳人数',
  `is_exit`    TINYINT(1)   NOT NULL DEFAULT 0,
  `status`     TINYINT(1)   NOT NULL DEFAULT 1 COMMENT '1=可通行 0=封闭',
  `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,
  `updated_at` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_node_key` (`node_key`),
  KEY `idx_floor` (`floor_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='建筑图-节点';

-- ============================================
-- 建筑图边表：节点之间的连接关系
-- ============================================
CREATE TABLE IF NOT EXISTS `graph_edge` (
  `id`            INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `from_node_key` VARCHAR(64) NOT NULL COMMENT '起始节点key',
  `to_node_key`   VARCHAR(64) NOT NULL COMMENT '目标节点key',
  `distance`      DECIMAL(8,2) NOT NULL COMMENT '物理距离(米)',
  `base_weight`   DECIMAL(8,2) NOT NULL DEFAULT 1.0 COMMENT '基础权重(1.0=正常)',
  `width`         DECIMAL(5,2) DEFAULT NULL COMMENT '通道宽度(米)',
  `bidirectional` TINYINT(1)  NOT NULL DEFAULT 1 COMMENT '是否双向',
  `status`        TINYINT(1)  NOT NULL DEFAULT 1 COMMENT '1=可通行 0=封闭',
  `created_at`    DATETIME DEFAULT CURRENT_TIMESTAMP,
  `updated_at`    DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_edge` (`from_node_key`, `to_node_key`),
  KEY `idx_from` (`from_node_key`),
  KEY `idx_to` (`to_node_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='建筑图-边';

-- ============================================
-- 传感器-节点关联表
-- ============================================
CREATE TABLE IF NOT EXISTS `sensor_node_binding` (
  `id`         INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `sensor_id`  INT UNSIGNED NOT NULL,
  `node_key`   VARCHAR(64)  NOT NULL COMMENT '关联节点',
  `influence_radius` DECIMAL(5,2) DEFAULT 5.0 COMMENT '影响半径(米)',
  PRIMARY KEY (`id`),
  KEY `idx_sensor` (`sensor_id`),
  KEY `idx_node` (`node_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='传感器-节点绑定';

-- ============================================
-- 算法参数配置表
-- ============================================
CREATE TABLE IF NOT EXISTS `algorithm_config` (
  `id`          INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `algorithm`   ENUM('aco','pso','sa','ga') NOT NULL,
  `name`        VARCHAR(64) NOT NULL COMMENT '配置名称',
  `params_json` JSON NOT NULL COMMENT '算法参数JSON',
  `is_default`  TINYINT(1) NOT NULL DEFAULT 0,
  `created_at`  DATETIME DEFAULT CURRENT_TIMESTAMP,
  `updated_at`  DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uk_algo_name` (`algorithm`, `name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='算法参数配置';

-- ============================================
-- 种子数据：商城楼层图
-- ============================================

-- 1F 节点
INSERT INTO `graph_node` (`floor_id`,`node_key`,`name`,`node_type`,`x`,`y`,`z`,`is_exit`) VALUES
(1, 'f1-lobby',      '1F 大厅',       'room',     0,    0,    0,   0),
(1, 'f1-shop-a',     '1F 商铺A',      'room',    -8,    0,   -5,   0),
(1, 'f1-shop-b',     '1F 商铺B',      'room',    -8,    0,    2,   0),
(1, 'f1-shop-c',     '1F 商铺C',      'room',     5,    0,   -5,   0),
(1, 'f1-shop-d',     '1F 商铺D',      'room',     5,    0,    2,   0),
(1, 'f1-cor-w',      '1F 西通道口',   'corridor', -8,    0,    0,   0),
(1, 'f1-cor-e',      '1F 东通道口',   'corridor',  8,    0,    0,   0),
(1, 'f1-cor-n',      '1F 北通道口',   'corridor',  0,    0,    7,   0),
(1, 'f1-cor-s',      '1F 南通道口',   'corridor',  0,    0,   -7,   0),
(1, 'f1-exit-east',  '1F 东门',       'exit',     10,    0,    0,   1),
(1, 'f1-exit-west',  '1F 西门',       'exit',    -10,    0,    0,   1),
(1, 'f1-exit-south', '1F 南门',       'exit',      0,    0,  -9,    1),
(1, 'f1-stair-east', '东侧楼梯-1F',  'stair',     9,    0,    6,   0),
(1, 'f1-stair-west', '西侧楼梯-1F',  'stair',    -9,    0,    6,   0),
(1, 'f1-elevator',   '电梯-1F',       'elevator',  0,    0,    6,   0);

-- 2F 节点
INSERT INTO `graph_node` (`floor_id`,`node_key`,`name`,`node_type`,`x`,`y`,`z`,`is_exit`) VALUES
(2, 'f2-office-a',   '2F 办公室A',    'room',    -7,   4.5,  -4,   0),
(2, 'f2-office-b',   '2F 办公室B',    'room',     7,   4.5,  -4,   0),
(2, 'f2-meeting',    '2F 会议室',     'room',     0,   4.5,  -4,   0),
(2, 'f2-cor-main',   '2F 主通道',     'corridor',  0,   4.5,   0,   0),
(2, 'f2-cor-w',      '2F 西通道口',   'corridor', -8,   4.5,   0,   0),
(2, 'f2-cor-e',      '2F 东通道口',   'corridor',  8,   4.5,   0,   0),
(2, 'f2-stair-east', '东侧楼梯-2F',  'stair',     9,   4.5,   6,   0),
(2, 'f2-stair-west', '西侧楼梯-2F',  'stair',    -9,   4.5,   6,   0),
(2, 'f2-elevator',   '电梯-2F',       'elevator',  0,   4.5,   6,   0);

-- 3F 节点
INSERT INTO `graph_node` (`floor_id`,`node_key`,`name`,`node_type`,`x`,`y`,`z`,`is_exit`) VALUES
(3, 'f3-hall',        '3F 展厅',      'room',     0,    9,   -3,   0),
(3, 'f3-server',      '3F 机房',      'room',    -7,    9,    0,   0),
(3, 'f3-lounge',      '3F 休息区',    'room',     7,    9,    0,   0),
(3, 'f3-cor-main',    '3F 主通道',    'corridor',  0,    9,    0,   0),
(3, 'f3-stair-east',  '东侧楼梯-3F', 'stair',     9,    9,    6,   0),
(3, 'f3-stair-west',  '西侧楼梯-3F', 'stair',    -9,    9,    6,   0),
(3, 'f3-elevator',    '电梯-3F',      'elevator',  0,    9,    6,   0);

-- 1F 边 (通道连接)
INSERT INTO `graph_edge` (`from_node_key`,`to_node_key`,`distance`,`base_weight`,`width`,`bidirectional`) VALUES
('f1-lobby',      'f1-cor-w',      8.0,  1.0, 3.0, 1),
('f1-lobby',      'f1-cor-e',      8.0,  1.0, 3.0, 1),
('f1-lobby',      'f1-cor-n',      7.0,  1.0, 2.5, 1),
('f1-lobby',      'f1-cor-s',      7.0,  1.0, 2.5, 1),
('f1-cor-w',      'f1-shop-a',     5.0,  1.0, 2.0, 1),
('f1-cor-w',      'f1-shop-b',     2.0,  1.0, 2.0, 1),
('f1-cor-w',      'f1-exit-west',  2.0,  1.0, 3.0, 1),
('f1-cor-e',      'f1-shop-c',     5.0,  1.0, 2.0, 1),
('f1-cor-e',      'f1-shop-d',     2.0,  1.0, 2.0, 1),
('f1-cor-e',      'f1-exit-east',  2.0,  1.0, 3.0, 1),
('f1-cor-s',      'f1-exit-south', 2.0,  1.0, 3.0, 1),
('f1-cor-n',      'f1-stair-east', 9.1,  1.0, 2.0, 1),
('f1-cor-n',      'f1-stair-west', 9.1,  1.0, 2.0, 1),
('f1-cor-n',      'f1-elevator',   1.0,  1.0, 2.0, 1),
('f1-cor-e',      'f1-stair-east', 6.1,  1.0, 2.0, 1),
('f1-cor-w',      'f1-stair-west', 6.1,  1.0, 2.0, 1);

-- 2F 边
INSERT INTO `graph_edge` (`from_node_key`,`to_node_key`,`distance`,`base_weight`,`width`,`bidirectional`) VALUES
('f2-cor-main',   'f2-cor-w',      8.0,  1.0, 3.0, 1),
('f2-cor-main',   'f2-cor-e',      8.0,  1.0, 3.0, 1),
('f2-cor-main',   'f2-meeting',    4.0,  1.0, 2.0, 1),
('f2-cor-w',      'f2-office-a',   4.1,  1.0, 2.0, 1),
('f2-cor-e',      'f2-office-b',   4.1,  1.0, 2.0, 1),
('f2-cor-e',      'f2-stair-east', 6.1,  1.0, 2.0, 1),
('f2-cor-w',      'f2-stair-west', 6.1,  1.0, 2.0, 1),
('f2-cor-main',   'f2-elevator',   6.0,  1.0, 2.0, 1);

-- 3F 边
INSERT INTO `graph_edge` (`from_node_key`,`to_node_key`,`distance`,`base_weight`,`width`,`bidirectional`) VALUES
('f3-cor-main',   'f3-hall',       3.0,  1.0, 2.5, 1),
('f3-cor-main',   'f3-server',     7.0,  1.0, 2.0, 1),
('f3-cor-main',   'f3-lounge',     7.0,  1.0, 2.0, 1),
('f3-cor-main',   'f3-stair-east', 9.5,  1.0, 2.0, 1),
('f3-cor-main',   'f3-stair-west', 9.5,  1.0, 2.0, 1),
('f3-cor-main',   'f3-elevator',   6.0,  1.0, 2.0, 1),
('f3-lounge',     'f3-stair-east', 6.3,  1.0, 2.0, 1),
('f3-server',     'f3-stair-west', 6.3,  1.0, 2.0, 1);

-- 楼梯跨楼层连接
INSERT INTO `graph_edge` (`from_node_key`,`to_node_key`,`distance`,`base_weight`,`width`,`bidirectional`) VALUES
('f1-stair-east', 'f2-stair-east', 5.0,  1.2, 1.5, 1),
('f1-stair-west', 'f2-stair-west', 5.0,  1.2, 1.5, 1),
('f1-elevator',   'f2-elevator',   3.0,  1.0, 2.0, 1),
('f2-stair-east', 'f3-stair-east', 5.0,  1.2, 1.5, 1),
('f2-stair-west', 'f3-stair-west', 5.0,  1.2, 1.5, 1),
('f2-elevator',   'f3-elevator',   3.0,  1.0, 2.0, 1);

-- 传感器-节点绑定种子数据
INSERT INTO `sensor_node_binding` (`sensor_id`,`node_key`,`influence_radius`) VALUES
(1, 'f1-lobby', 5.0),
(2, 'f2-cor-main', 5.0);

-- 算法默认参数
INSERT INTO `algorithm_config` (`algorithm`,`name`,`params_json`,`is_default`) VALUES
('aco', 'default', '{"ant_count":30,"alpha":1.0,"beta":3.0,"rho":0.3,"Q":100,"max_iterations":200,"danger_weight":10.0}', 1);
