import os
import subprocess
import textwrap

class terminal:
    def __init__(self):
        self.out = ''
        self.err = ''
        self.record = ''
        self.pref = '>>'
        self.cx = 0
        self.sy = 0
        self.hi = -1
        self.s = ''
        self.history = []
        self.p = None
        self.filename = ''
    def command(self, inpt):
        self.history.append(inpt)
        self.record += self.pref + inpt + '\n'
        pref = 'source ~/.bash_profile && f="{}" && '.format(self.filename)
        #if len(inpt) > 2 and inpt[:2] == './' : pref = ''
        process = subprocess.Popen(pref + inpt, shell=True,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
        self.p = process
        
        out, err = process.communicate()
        out = out.decode('utf-8')
        err = err.decode('utf-8')
        self.out += out
        self.err += err
        self.record += out + err + '\n'
        if inpt.strip() == 'clear':
            self.record = ''
        self.record = self.record[-10000:]
        return out, err
    def run(self):
        result = self.command(self.s)
        self.s = ''
        self.cx = 0
        self.sy = 0
        return result
    def kill(self):
        if self.p != None:
            self.p.kill()
            self.p = None
    def getlines(self, w):
        sp = self.record.split('\n')
        result = []
        for s in sp:
            for t in textwrap.fill(s, w).split('\n'):
                result.append(t)
        return result
