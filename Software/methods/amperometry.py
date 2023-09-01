# This Python file uses the following encoding: utf-8

from PySide2.QtCore import QObject, Slot

class Amperometry(QObject):
    def __init__(self, lmp_gain=1, pre_stepV=0, quietTime=0, v1=0, t1=0, v2=0, t2=1, samples=1, ranges = 3, set_to_zero=False):
        self.lmp_gain = lmp_gain
        self.pre_stepV = pre_stepV
        self.quietTime = quietTime
        self.v1 = v1
        self.t1 = t1
        self.v2 = v2
        self.t2 = t2
        self.samples = samples
        self.ranges = ranges
        self.set_to_zero = set_to_zero

    def get_lmp_gain(self):
        return self.lmp_gain

    @Slot(str)
    def set_lmp_gain(self, value):
        print(f"Set lmp gain to: {value}, {type(value)}")
        self.lmp_gain = int(value)

    def get_pre_stepV(self):
        return self.pre_stepV

    @Slot(int)
    def set_pre_stepV(self, value):
        print(f"Set Initial Potential to: {value}")
        self.pre_stepV = value

    def get_quietTime(self):
        return self.quietTime

    @Slot(int)
    def set_quietTime(self, value):
        print(f"Set Quiet Time to: {value}")
        self.quietTime = value

    def get_v1(self):
        return self.v1

    @Slot(int)
    def set_v1(self, value):
        print(f"Set First Step E to: {value}")
        self.v1 = value

    def get_t1(self):
        return self.t1

    @Slot(int)
    def set_t1(self, value):
        print(f"Set First Step Time to: {value}")
        self.t1 = value

    def get_v2(self):
        return self.v2

    @Slot(int)
    def set_v2(self, value):
        print(f"Set Second Step E to: {value}")
        self.v2 = value

    def get_t2(self):
        return self.t2

    @Slot(int)
    def set_t2(self, value):
        print(f"Set Second Step Time to: {value}")
        self.t2 = value

    def get_samples(self):
        return self.samples

    @Slot(int)
    def set_samples(self, value):
        print(f"Set Samples to: {value}")
        self.samples = value

    def get_ranges(self):
        return self.ranges

    @Slot(int)
    def set_ranges(self, value):
        self.ranges = (value+1)*3
        print(f"Set Range to: {self.ranges}")

    def get_set_to_zero(self):
        return self.set_to_zero

    @Slot(int)
    def set_set_to_zero(self, value):
        print(f"Set set_to_zero to: {value}, {bool(value)}")
        self.set_to_zero = bool(value)


#    def verifyData(self):
#        if self.get_lmp_gain() <  0 or self.get_lmp_gain() >  7:
#            return False
#        elif self.get_cycles() <  1 or self.get_cycles() >  50:
#            return False
#        elif self.get_start_voltage() <  -650 or self.get_start_voltage() >  650:
#            return False
#        elif self.get_end_voltage() <  -650 or self.get_end_voltage() >  650:
#            return False
#        elif self.get_vertex_1() <  -650 or self.get_vertex_1() >  650:
#            return False
#        elif self.get_vertex_2() <  -650 or self.get_vertex_2() >  650:
#            return False
#        elif int(self.get_step_voltage()) <  1 or int(self.get_step_voltage()) >  650:
#            return False
#        elif self.get_scan_rate() <  1 or self.get_scan_rate() >  1000:
#            return False
##        if self.get_set_to_zero <  1 or self.get_set_to_zero >  2
##            return False
#        else:
#            return True
