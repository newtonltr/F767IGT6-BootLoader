#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
文件传输客户端
用于向单片机传输bin文件的TCP客户端工具
"""

import socket
import struct
import os
import sys
import time
from typing import Optional, Tuple

# 常量定义
MAX_FILE_SIZE = 200 * 1024  # 200KB限制
PACKET_DATA_SIZE = 512      # 每包数据大小
CONNECTION_TIMEOUT = 10     # 连接超时（秒）
MAX_CONNECT_RETRIES = 3     # 最大连接重试次数
MAX_SEND_RETRIES = 10       # 最大发送重试次数
SERVER_PORT = 7000          # 默认服务器端口
DEFAULT_SERVER_IP = "192.168.0.232"  # 默认服务器IP地址

# 服务器响应消息
RESPONSE_CONNECTED = b"client connected \r\n"
RESPONSE_SUCCESS = b"recv success \r\n"
RESPONSE_CRC_ERROR = b"crc error \r\n"
RESPONSE_INCOMPLETE = b"recv incomplete \r\n"
RESPONSE_START_PROGRAM = b"start program \r\n"
RESPONSE_PROGRAM_COMPLETE = b"program complete \r\n"

class FileTransferClient:
    """文件传输客户端类"""
    
    # CRC查找表 - 与C代码中auchCRCHi相同
    CRC_HI_TABLE = [
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
        0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
        0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
        0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
        0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
        0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
        0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
        0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
        0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
        0x40
    ]
    
    # CRC查找表 - 与C代码中auchCRCLo相同
    CRC_LO_TABLE = [
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
        0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
        0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
        0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
        0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
        0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
        0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
        0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
        0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
        0x40
    ]
    
    def __init__(self):
        """初始化客户端"""
        self.socket: Optional[socket.socket] = None
        self.server_ip: str = ""
        self.server_port: int = SERVER_PORT
        
    def run(self):
        """运行主程序"""
        print("=== 文件传输客户端 ===")
        print("用于向单片机传输bin文件")
        print()
        
        # 步骤1: 获取连接信息并建立连接
        if not self._connect_to_server():
            print("程序退出")
            return
            
        # 步骤2: 获取文件路径并验证
        file_path = self._get_and_validate_file()
        if not file_path:
            print("程序退出")
            return
            
        # 步骤3: 开始文件传输
        try:
            self._transfer_file(file_path)
        finally:
            self._cleanup()
    
    def _cleanup(self):
        """清理资源"""
        if self.socket:
            try:
                self.socket.close()
                print("连接已关闭")
            except:
                pass
            self.socket = None
    
    def crc_compute_lsb(self, data: bytes) -> int:
        """
        计算CRC16校验码，与C代码中crc_compute_lsb函数完全相同
        返回值格式: ((uchCRCLo << 8) | (uchCRCHi))
        """
        uch_crc_hi = 0xFF
        uch_crc_lo = 0xFF
        
        for byte in data:
            u_index = uch_crc_hi ^ byte
            uch_crc_hi = uch_crc_lo ^ self.CRC_HI_TABLE[u_index]
            uch_crc_lo = self.CRC_LO_TABLE[u_index]
        
        return ((uch_crc_lo << 8) | uch_crc_hi)
    
    def _get_connection_info(self) -> bool:
        """获取连接信息"""
        try:
            # 获取IP地址
            while True:
                ip = input(f"请输入服务器IP地址 (默认{DEFAULT_SERVER_IP}): ").strip()
                if not ip:
                    self.server_ip = DEFAULT_SERVER_IP
                    break
                if self._validate_ip(ip):
                    self.server_ip = ip
                    break
                print("请输入有效的IP地址\n")
            
            # 获取端口（可选，默认7000）
            while True:
                port_input = input(f"请输入端口号 (默认{SERVER_PORT}): ").strip()
                if not port_input:
                    self.server_port = SERVER_PORT
                    break
                try:
                    port = int(port_input)
                    if 1 <= port <= 65535:
                        self.server_port = port
                        break
                    else:
                        print("端口号必须在1-65535之间\n")
                except ValueError:
                    print("请输入有效的端口号\n")
            
            print(f"连接目标: {self.server_ip}:{self.server_port}")
            return True
            
        except KeyboardInterrupt:
            print("\n用户取消操作")
            return False
    
    def _validate_ip(self, ip: str) -> bool:
        """验证IP地址格式"""
        try:
            parts = ip.split('.')
            if len(parts) != 4:
                return False
            for part in parts:
                if not 0 <= int(part) <= 255:
                    return False
            return True
        except ValueError:
            return False
    
    def _attempt_connection(self) -> bool:
        """尝试单次连接"""
        try:
            if self.socket:
                self.socket.close()
            
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(CONNECTION_TIMEOUT)
            
            print(f"正在连接到 {self.server_ip}:{self.server_port}...")
            self.socket.connect((self.server_ip, self.server_port))
            
            # 等待连接确认消息
            try:
                response = self.socket.recv(1024)
                if response == RESPONSE_CONNECTED:
                    print("连接成功！")
                    return True
                else:
                    print(f"服务器响应异常: {response}")
                    return False
            except socket.timeout:
                print("等待服务器确认超时")
                return False
                
        except socket.timeout:
            print("连接超时")
            return False
        except ConnectionRefusedError:
            print("连接被拒绝")
            return False
        except Exception as e:
            print(f"连接失败: {e}")
            return False
    
    def _ask_continue_connection(self) -> bool:
        """询问用户是否继续连接"""
        while True:
            try:
                choice = input("是否继续尝试连接？(y/n): ").strip().lower()
                if choice in ['y', 'yes', '是']:
                    return True
                elif choice in ['n', 'no', '否']:
                    return False
                else:
                    print("请输入 y 或 n")
            except KeyboardInterrupt:
                print("\n用户取消操作")
                return False
    
    def _connect_to_server(self) -> bool:
        """连接到服务器，返回是否成功"""
        # 获取连接信息
        if not self._get_connection_info():
            return False
        
        print()
        
        # 连接循环
        while True:
            # 尝试连接，最多重试3次
            for attempt in range(1, MAX_CONNECT_RETRIES + 1):
                print(f"连接尝试 {attempt}/{MAX_CONNECT_RETRIES}")
                
                if self._attempt_connection():
                    return True
                
                if attempt < MAX_CONNECT_RETRIES:
                    print(f"等待1秒后重试...\n")
                    time.sleep(1)
                else:
                    print(f"连续{MAX_CONNECT_RETRIES}次连接失败\n")
            
            # 询问是否继续
            if not self._ask_continue_connection():
                return False
            
            print()
        
    def _validate_file(self, file_path: str) -> bool:
        """验证文件是否符合要求"""
        # 检查文件是否存在
        if not os.path.exists(file_path):
            print(f"错误: 文件不存在: {file_path}")
            return False
        
        # 检查是否是bin文件
        if not file_path.lower().endswith('.bin'):
            print(f"错误: 文件必须是.bin格式，当前文件: {file_path}")
            return False
        
        # 检查文件大小
        file_size = os.path.getsize(file_path)
        if file_size > MAX_FILE_SIZE:
            print(f"错误: 文件大小超过限制")
            print(f"当前大小: {file_size} 字节 ({file_size/1024:.1f} KB)")
            print(f"最大限制: {MAX_FILE_SIZE} 字节 ({MAX_FILE_SIZE/1024:.0f} KB)")
            return False
        
        print(f"文件验证通过: {os.path.basename(file_path)}")
        print(f"文件大小: {file_size} 字节 ({file_size/1024:.1f} KB)")
        return True
    
    def _get_and_validate_file(self) -> Optional[str]:
        """获取并验证文件路径"""
        while True:
            try:
                file_path = input("请输入bin文件路径: ").strip()
                
                # 处理路径中的引号
                file_path = file_path.strip('"').strip("'")
                
                if not file_path:
                    print("错误: 请输入有效的文件路径")
                    continue
                
                if self._validate_file(file_path):
                    return file_path
                else:
                    print()
                    
            except KeyboardInterrupt:
                print("\n用户取消操作")
                return None
            except Exception as e:
                print(f"输入错误: {e}")
                print()
        
    def _prepare_packets(self, file_path: str) -> list:
        """将文件分包处理"""
        packets = []
        
        with open(file_path, 'rb') as f:
            file_data = f.read()
        
        total_bytes = len(file_data)
        total_packs = (total_bytes + PACKET_DATA_SIZE - 1) // PACKET_DATA_SIZE  # 向上取整
        
        print(f"文件总大小: {total_bytes} 字节")
        print(f"预计分包数量: {total_packs} 包")
        print()
        
        for i in range(total_packs):
            start_idx = i * PACKET_DATA_SIZE
            end_idx = min(start_idx + PACKET_DATA_SIZE, total_bytes)
            
            # 获取这一包的数据
            packet_data = file_data[start_idx:end_idx]
            data_size = len(packet_data)
            
            # 如果数据不足512字节，补零
            if len(packet_data) < PACKET_DATA_SIZE:
                packet_data += b'\x00' * (PACKET_DATA_SIZE - len(packet_data))
            
            # 准备数据包信息
            packet_info = {
                'pack_index': i + 1,  # 从1开始
                'total_bytes': total_bytes,
                'total_packs': total_packs,
                'data_size': data_size,
                'data': packet_data
            }
            
            packets.append(packet_info)
        
        return packets
    
    def _create_packet_bytes(self, packet_info: dict) -> bytes:
        """创建数据包的字节流"""
        # 不包含CRC的数据部分
        packet_without_crc = struct.pack('<IIII512s', 
                                        packet_info['pack_index'],
                                        packet_info['total_bytes'],
                                        packet_info['total_packs'],
                                        packet_info['data_size'],
                                        packet_info['data'])
        
        # 计算CRC16校验码
        crc16 = self.crc_compute_lsb(packet_without_crc)
        
        # 完整的数据包（包含CRC）
        complete_packet = packet_without_crc + struct.pack('<H', crc16)
        
        return complete_packet
    
    def _send_packet_with_retry(self, packet_info: dict) -> Tuple[bool, bool]:
        """发送单个数据包，带重试机制
        返回: (是否成功, 是否是最后一包且接收到start_program)
        """
        packet_bytes = self._create_packet_bytes(packet_info)
        pack_index = packet_info['pack_index']
        total_packs = packet_info['total_packs']
        is_last_packet = (pack_index == total_packs)
        
        for retry in range(1, MAX_SEND_RETRIES + 1):
            try:
                # 发送数据包
                self.socket.sendall(packet_bytes)
                
                # 等待服务器响应
                response = self.socket.recv(1024)
                
                if response == RESPONSE_SUCCESS:
                    # 发送成功后等待10ms，避免发送过快导致服务器处理不过来
                    time.sleep(0.01)
                    return True, False
                elif response == RESPONSE_START_PROGRAM and is_last_packet:
                    # 最后一包收到start_program响应
                    return True, True
                elif response == RESPONSE_CRC_ERROR:
                    print(f"    重试 {retry}/{MAX_SEND_RETRIES}: 单片机回复: {response.decode('utf-8', errors='ignore').strip()}")
                elif response == RESPONSE_INCOMPLETE:
                    print(f"    重试 {retry}/{MAX_SEND_RETRIES}: 单片机回复: {response.decode('utf-8', errors='ignore').strip()}")
                else:
                    print(f"    重试 {retry}/{MAX_SEND_RETRIES}: 单片机回复: {response.decode('utf-8', errors='ignore').strip()}")
                
                if retry < MAX_SEND_RETRIES:
                    time.sleep(0.1)  # 重试间隔100ms
                    
            except socket.timeout:
                print(f"    重试 {retry}/{MAX_SEND_RETRIES}: 响应超时")
                if retry < MAX_SEND_RETRIES:
                    time.sleep(0.1)
            except ConnectionResetError:
                print(f"    重试 {retry}/{MAX_SEND_RETRIES}: 网络错误 - 连接被重置")
                return False, False  # 需要重连
            except BrokenPipeError:
                print(f"    重试 {retry}/{MAX_SEND_RETRIES}: 网络错误 - 连接断开")
                return False, False  # 需要重连
            except Exception as e:
                print(f"    重试 {retry}/{MAX_SEND_RETRIES}: 发送错误 {e}")
                if retry < MAX_SEND_RETRIES:
                    time.sleep(0.1)
        
        return False, False
    
    def _display_progress(self, current: int, total: int, status: str = ""):
        """显示传输进度"""
        percentage = (current / total) * 100
        bar_length = 50
        filled_length = int(bar_length * current // total)
        bar = '█' * filled_length + '-' * (bar_length - filled_length)
        
        print(f"\r进度: [{bar}] {percentage:6.2f}% ({current}/{total}) {status}", end='', flush=True)
    
    def _ask_transfer_choice(self) -> str:
        """询问传输失败后的选择"""
        while True:
            try:
                print("\n传输失败，请选择:")
                print("1. 继续当前传输")
                print("2. 重新开始传输") 
                print("3. 退出程序")
                choice = input("请输入选择 (1/2/3): ").strip()
                
                if choice == '1':
                    return 'continue'
                elif choice == '2':
                    return 'restart'
                elif choice == '3':
                    return 'exit'
                else:
                    print("请输入 1、2 或 3")
            except KeyboardInterrupt:
                print("\n用户取消操作")
                return 'exit'
    
    def _reconnect_if_needed(self) -> bool:
        """在需要时重新连接"""
        print("\n尝试重新连接...")
        
        # 连接循环
        while True:
            # 尝试连接，最多重试3次
            for attempt in range(1, MAX_CONNECT_RETRIES + 1):
                print(f"重连尝试 {attempt}/{MAX_CONNECT_RETRIES}")
                
                if self._attempt_connection():
                    return True
                
                if attempt < MAX_CONNECT_RETRIES:
                    print(f"等待1秒后重试...\n")
                    time.sleep(1)
                else:
                    print(f"连续{MAX_CONNECT_RETRIES}次重连失败\n")
            
            # 询问是否继续
            if not self._ask_continue_connection():
                return False
            
            print()
    
    def _wait_for_programming_complete(self, file_size: int):
        """等待单片机完成程序烧录"""
        print("\n✓ 文件传输完成！开始下载到APP区域...")
        print("正在等待单片机完成程序烧录...")
        
        # 设置较短的接收超时，避免长时间阻塞
        original_timeout = self.socket.gettimeout()
        self.socket.settimeout(5.0)  # 增加超时时间，因为不需要频繁检查进度
        
        # 设置总超时时间（60秒）
        start_time = time.time()
        total_timeout = 60.0
        
        try:
            while True:
                current_time = time.time()
                
                # 检查总超时
                if current_time - start_time > total_timeout:
                    print(f"\n等待烧录完成超时（{total_timeout}秒）")
                    break
                
                try:
                    # 接收单片机发送的数据
                    response = self.socket.recv(1024)
                    
                    if not response:
                        print("\n连接断开")
                        break
                    
                    # 检查是否是程序烧录完成消息
                    if response == RESPONSE_PROGRAM_COMPLETE:
                        print(f"\n✓ 程序烧录完成！IAP流程结束")
                        print("烧写完成，跳转到应用程序")
                        break
                    
                    else:
                        # 其他消息直接显示（如start_program等）
                        try:
                            msg = response.decode('utf-8', errors='ignore').strip()
                            if msg:
                                print(f"  单片机消息: {msg}")
                            else:
                                print(f"  接收到数据: {response.hex()}")
                        except Exception as e:
                            print(f"  消息解码错误: {e}, 原始数据: {response.hex()}")
                
                except socket.timeout:
                    # 短暂超时是正常的，继续等待
                    continue
                    
                except ConnectionResetError:
                    print("\n连接被重置")
                    break
                    
                except Exception as e:
                    print(f"\n等待烧录过程中发生错误: {e}")
                    break
                    
        finally:
            # 恢复原始超时设置
            if original_timeout is not None:
                self.socket.settimeout(original_timeout)
            else:
                self.socket.settimeout(None)
    
    def _transfer_file(self, file_path: str):
        """传输文件"""
        start_time = time.time()
        
        while True:  # 支持重新开始传输
            # 准备数据包
            print("正在准备数据包...")
            packets = self._prepare_packets(file_path)
            
            print("开始传输文件...")
            success_count = 0
            
            # 逐包传输
            programming_started = False
            for i, packet_info in enumerate(packets):
                pack_index = packet_info['pack_index']
                total_packs = packet_info['total_packs']
                
                # 显示当前进度
                self._display_progress(i, total_packs, f"正在发送第 {pack_index} 包")
                
                # 发送数据包
                success, start_programming = self._send_packet_with_retry(packet_info)
                if success:
                    success_count += 1
                    self._display_progress(success_count, total_packs, "✓ 成功")
                    print()  # 换行，使成功信息更清晰
                    
                    # 检查是否是最后一包且开始烧录
                    if start_programming:
                        programming_started = True
                        break
                else:
                    # 发送失败，检查是否需要重连
                    print(f"\n第 {pack_index} 包发送失败")
                    
                    # 尝试重连
                    if not self._reconnect_if_needed():
                        print("重连失败，传输中止")
                        return
                    
                    # 询问用户选择
                    choice = self._ask_transfer_choice()
                    if choice == 'continue':
                        print("继续当前传输...")
                        # 重试当前包
                        retry_success, retry_start_programming = self._send_packet_with_retry(packet_info)
                        if retry_success:
                            success_count += 1
                            self._display_progress(success_count, total_packs, "✓ 重试成功")
                            print()  # 换行，使成功信息更清晰
                            
                            # 检查是否是最后一包且开始烧录
                            if retry_start_programming:
                                programming_started = True
                                break
                        else:
                            print(f"第 {pack_index} 包仍然发送失败")
                            choice = self._ask_transfer_choice()
                            if choice == 'restart':
                                print("重新开始传输...")
                                break
                            elif choice == 'exit':
                                print("用户选择退出")
                                return
                    elif choice == 'restart':
                        print("重新开始传输...")
                        break
                    elif choice == 'exit':
                        print("用户选择退出")
                        return
            else:
                # 所有包传输完成但没有收到start_programming
                if not programming_started:
                    end_time = time.time()
                    duration = end_time - start_time
                    
                    print(f"\n\n✓ 文件传输完成！")
                    print(f"总用时: {duration:.2f} 秒")
                    print(f"传输速度: {len(packets) * PACKET_DATA_SIZE / duration / 1024:.2f} KB/s")
                    return
            
            # 如果开始了烧录流程，等待完成
            if programming_started:
                self._wait_for_programming_complete(os.path.getsize(file_path))
                
                end_time = time.time()
                duration = end_time - start_time
                
                print(f"\n整个IAP流程完成！")
                print(f"总用时: {duration:.2f} 秒")
                print(f"传输速度: {len(packets) * PACKET_DATA_SIZE / duration / 1024:.2f} KB/s")
                return

def main():
    """主函数"""
    try:
        client = FileTransferClient()
        client.run()
    except KeyboardInterrupt:
        print("\n程序被用户中断")
    except Exception as e:
        print(f"程序发生错误: {e}")

if __name__ == "__main__":
    main() 