from PySide2.QtCore import QObject, Slot

class CyclicVoltammetry(QObject):
    def __init__(self, lmp_gain=1, cycles=0, start_voltage=0, end_voltage=0, vertex_1=0, vertex_2=0, step_voltage=1, scan_rate=1, set_to_zero=False):
        self.lmp_gain = lmp_gain
        self.cycles = cycles
        self.start_voltage = start_voltage
        self.end_voltage = end_voltage
        self.vertex_1 = vertex_1
        self.vertex_2 = vertex_2
        self.step_voltage = step_voltage
        self.scan_rate = scan_rate
        self.set_to_zero = set_to_zero

    def get_lmp_gain(self):
        return self.lmp_gain
    
    @Slot(str)
    def set_lmp_gain(self, value):
        print(f"Set lmp gain to: {value}, {type(value)}")
        self.lmp_gain = int(value)

    def get_cycles(self):
        return self.cycles

    @Slot(int)
    def set_cycles(self, value):
        print(f"Set cycles to: {value}")
        self.cycles = value
    
    def get_start_voltage(self):
        return self.start_voltage

    @Slot(int)
    def set_start_voltage(self, value):
        print(f"Set start_voltage to: {value}")
        self.start_voltage = value
  
    def get_end_voltage(self):
        return self.end_voltage

    @Slot(int)
    def set_end_voltage(self, value):
        print(f"Set end_voltage to: {value}")
        self.end_voltage = value

    def get_vertex_1(self):
        return self.vertex_1

    @Slot(int)
    def set_vertex_1(self, value):
        print(f"Set vertex_1 to: {value}")
        self.vertex_1 = value
    
    def get_vertex_2(self):
        return self.vertex_2

    @Slot(int)
    def set_vertex_2(self, value):
        print(f"Set vertex_2 to: {value}")
        self.vertex_2 = value

    def get_step_voltage(self):
        return self.step_voltage

    @Slot(int)
    def set_step_voltage(self, value):
        print(f"Set step_voltage to: {value}")
        self.step_voltage = value

    def get_scan_rate(self):
        return self.scan_rate

    @Slot(int)
    def set_scan_rate(self, value):
        print(f"Set scan_rate to: {value}")
        self.scan_rate = value

    def get_set_to_zero(self):
        return self.set_to_zero
    
    @Slot(int)
    def set_set_to_zero(self, value):
        print(f"Set set_to_zero to: {value}, {bool(value)}")
        self.set_to_zero = bool(value)


    def verifyData(self):
        if self.get_lmp_gain() <  0 or self.get_lmp_gain() >  7:
            return False
        elif self.get_cycles() <  1 or self.get_cycles() >  50:
            return False
        elif self.get_start_voltage() <  -650 or self.get_start_voltage() >  650:
            return False
        elif self.get_end_voltage() <  -650 or self.get_end_voltage() >  650:
            return False
        elif self.get_vertex_1() <  -650 or self.get_vertex_1() >  650:
            return False
        elif self.get_vertex_2() <  -650 or self.get_vertex_2() >  650:
            return False
        elif int(self.get_step_voltage()) <  1 or int(self.get_step_voltage()) >  650:
            return False
        elif self.get_scan_rate() <  1 or self.get_scan_rate() >  1000:
            return False
#        if self.get_set_to_zero <  1 or self.get_set_to_zero >  2
#            return False
        else:
            return True
