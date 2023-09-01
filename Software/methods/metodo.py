# This Python file uses the following encoding: utf-8
from PySide2.QtCore import QStateMachine, QState, QFinalState, QObject
from PySide2.QtWidgets import QApplication
from PySide2.QtCore import Signal as pyqtSignal, Slot as pyqtSlot


class Metodo(QObject):
    stateChanged = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.stateWait = QState()
        self.stateWait.setObjectName("Wait")
        self.stateWait.entered.connect(self.on_state_changed)

        self.stateCV = QState()
        self.stateCV.setObjectName("CV")
        self.stateCV.entered.connect(self.on_state_changed)

        self.stateAmp = QState()
        self.stateAmp.setObjectName("Amp")
        self.stateAmp.entered.connect(self.on_state_changed)

        self.stateWait.addTransition(self.stateWait.entered, self.stateCV)
        self.stateCV.addTransition(self.stateCV.entered, self.stateAmp)
        self.stateAmp.addTransition(self.stateAmp.entered, self.stateWait)

        self.finalState = QFinalState()

        self.machine = QStateMachine()
        self.machine.addState(self.stateWait)
        self.machine.addState(self.stateCV)
        self.machine.addState(self.stateAmp)
        self.machine.addState(self.finalState)
        self.machine.setInitialState(self.stateWait)

        self.machine.start()

    def on_state_changed(self):
        state_name = self.sender().objectName()
        self.stateChanged.emit(state_name)
