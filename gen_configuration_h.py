from rdma_config import get_net_config
from rdma_config import choose_link
from string import Template
import datetime
import os
import sys

def gen_configuration_h(path: str):
    link = choose_link(get_net_config())
    with open(os.path.join(path, 'Configuration.h.template'), 'r') as t:
        tmpl = Template(t.read())

    with open(os.path.join(path, 'Configuration.h'), 'w') as c:
        lines = []
        lines.append(tmpl.substitute(
            GENTIME=datetime.datetime.now().strftime("%m/%d/%Y, %H:%M:%S"),
            NETDEV=link['netdev'],
            IBDEV=link['link'],
            IBPORT=link['port']
        ))
        c.writelines(lines)


if __name__ == '__main__':
    cfg_h_path = sys.argv[1]
    gen_configuration_h(cfg_h_path)
