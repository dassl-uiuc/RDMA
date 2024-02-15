import ipaddress
import subprocess
import socket
import fcntl
import struct

def get_ip_address(ifname: str) -> str:
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        return socket.inet_ntoa(fcntl.ioctl(
            s.fileno(),
            0x8915,  # SIOCGIFADDR
            struct.pack('256s', bytes(ifname[:15], 'utf-8'))
        )[20:24])
    except Exception as e:
        print('Get ip address error: ', e)
        return None


def is_public_ip(ip: str) -> bool:
    try:
        # Parse the IP address
        ip_obj = ipaddress.ip_address(ip)
        # Check if the IP address is in any of the reserved ranges for private/internal IPs
        private_ranges = [
            ipaddress.ip_network('10.0.0.0/8'),
            ipaddress.ip_network('172.16.0.0/12'),
            ipaddress.ip_network('192.168.0.0/16'),
            ipaddress.ip_network('127.0.0.0/8'),
        ]
        for private_range in private_ranges:
            if ip_obj in private_range:
                return False  # Private IP
        # Check if the IP address is in the reserved range for loopback address
        if ip_obj.is_loopback:
            return False  # Loopback IP
        # Check if the IP address is in the reserved range for link-local address
        if ip_obj.is_link_local:
            return False  # Link-local IP
        # If the IP address doesn't match any private or reserved ranges, it's public
        return True
    except ValueError:
        # If the IP address is not valid, return None (indeterminate)
        return None


def get_net_config() -> 'list[dict]':
    try:
        result = subprocess.run("rdma link", shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='utf-8')
        lines = result.stdout.strip().split('\n')
    except subprocess.CompletedProcess as e:
        print('Get rdma link error: ', e)
    
    active_links: list[dict] = []
    for line in lines:
        info = line.split(' ')
        link = info[1].split('/')[0]
        port = info[1].split('/')[1]
        state = info[3]
        if state != 'ACTIVE':
            continue
        physical_state = info[5]
        if physical_state != 'LINK_UP':
            continue
        netdev = info[7]
        ip = get_ip_address(netdev)
        if ip is None:
            continue
        active_links.append({'link': link, 'port': port, 'netdev': netdev, 'ip': ip})
    
    return active_links


def choose_link(active_links: 'list[dict]') -> dict:
    if len(active_links) == 0:
        print('NO active RDMA link!')
        return None
    
    has_private_ip = False
    n = 0
    for link in active_links:
        link['glbl_idx'] = n
        if not is_public_ip(link['ip']):
            return link
        n += 1

    print("no internal faced interface, use the first active device")
    return link
