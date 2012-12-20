#!/usr/bin/env python3
# -*- encoding: utf-8 -*-

import sys
import copy
import configparser

from collections import OrderedDict

class Conf:
    def __init__(self):
        self._conf = configparser.ConfigParser()
        self._conf["hole"] = { }

    def set_listen_port(self, port, mode="slug"):
        self._conf["hole"]["{0}.port".format(mode)] = str(port)

    def get_listen_port(self, mode="slug"):
        return int(self.conf["hole"]["{mode}.port".format(mode=mode)])

    def set_timeout(self, timeout, mode="slug"):
        self._conf["hole"]["{0}.timeout".format(mode)] = str(timeout)

    @property
    def conf(self):
        return self._conf

    def to_dict(self):
        tmp = OrderedDict()
        for (section, values) in self._conf.items():
            for (key, val) in values.items():
                if section not in tmp:
                    tmp[section] = OrderedDict()
                tmp[section][key] = val
        return tmp

    def dump(self):
        import pprint
        pprint.pprint(self._conf)

    def commit(self, file=sys.stdout):
        self.conf.write(file)

    def clone(self):
        new_c = Conf()
        for (key, val) in self._conf.items():
            new_c._conf[key] = val
        return new_c


if __name__ == "__main__":
    c = Conf()
    c.set_timeout(1000)
    c.set_listen_port(5656)
    c.commit()


