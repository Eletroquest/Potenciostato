# Define a codificação utf-8 para este arquivo Python
import os
from pathlib import Path
import sys

# Importa classes e módulos do PySide2 para criar a interface gráfica
from PySide2.QtWidgets import QApplication, QWidget, QFileDialog, QMainWindow, QLabel, QGridLayout, QMessageBox
from PySide2.QtCore import QFile, Slot, QTimer, QDateTime, QCoreApplication, QTime, Qt, QState, QObject, QThread, Signal
from PySide2.QtCore import Signal as pyqtSignal, Slot as pyqtSlot
from PySide2.QtUiTools import QUiLoader
from PySide2.QtSerialPort import QSerialPort, QSerialPortInfo
from PySide2.QtGui import QFontDatabase, QColor, QPen, QBrush, QPixmap, QIcon
from qcustomplot_pyside2 import *  # Importa módulo personalizado
from methods import *  # Importa métodos personalizados

# Inicializa uma lista vazia para armazenar dados decodificados
list_decoded_data = []

# Define uma classe enum para representar estados do modo BLE
class mode_BLE():
    """
    Classe enum para representar estados do modo BLE.

    - `BLE_CONNECT`: Estado de conexão BLE.
    - `BLE_DISCONNECT`: Estado de desconexão BLE.
    - `BLE_DISABLE`: Estado desabilitado do BLE.
    """
    BLE_CONNECT = 1
    BLE_DISCONNECT = 2
    BLE_DISABLE = 3

# Define a classe principal da janela principal do aplicativo
class MainWindow(QMainWindow):
    def __init__(self):
        """
        Inicializa a janela principal do aplicativo.

        - Carrega a interface gráfica definida em um arquivo .ui.
        - Configuração inicial do gráfico.
        - Limpa a barra de menus e o widget de texto.
        - Inicializa variáveis e objetos necessários.
        - Configura a interface com os métodos de CyclicVoltammetry e Amperometry.
        - Configura a comunicação serial.
        - Define botões iniciais e estados.
        """
        super(MainWindow, self).__init__()

        # Carrega a interface gráfica definida em um arquivo .ui
        self.load_ui()

        # Configuração inicial do gráfico
        self.setup_chart()

        # Limpa a barra de menus e o widget de texto
        self.ui.menubar.clear()
        self.ui.textEdit.clear()

        self.dado = 0

        # Instância de métodos para CyclicVoltammetry e Amperometry
        self.cyclic_voltammetry = CyclicVoltammetry()
        self.connect_inicialCV()
        self.amperometry = Amperometry()
        self.connect_inicialAmp()
        self.connect_ui_signals()

        self.is_serial_port_connected = False
        self.connected_serial_port = QSerialPort()
        self.connected_serial_port.setBaudRate(QSerialPort.Baud115200)
        self.connected_serial_port.setDataBits(QSerialPort.Data8)
        self.connected_serial_port.setParity(QSerialPort.NoParity)
        self.connected_serial_port.setStopBits(QSerialPort.OneStop)
        self.available_serial_ports = []

        # Inicia a verificação das portas seriais disponíveis
        self.scan_serial_ports()
        self.serial_port_scan_timer = QTimer()
        self.serial_port_scan_timer.timeout.connect(self.scan_serial_ports)
        self.serial_port_scan_timer.start(1000)

        # Conecta o slot para ler dados da porta serial quando dados estão prontos
        self.connected_serial_port.readyRead.connect(self.read_serial_port_data)

        # Configura botões iniciais
        self.ui.startButton.setEnabled(False)
        self.ui.stopButton.setEnabled(False)

        self.csv_file = None
        self.csv_file_name = ""
        self.ui.outputFileName.setText(self.csv_file_name)
        self.ui.statusbar.showMessage("Configurando Voltametria ...")

        self.ui.textEdit.setReadOnly(True)

        # Controle de estados do método
        self.currentMedoto = self.ui.toolBox_metodo.currentIndex()
        self.runnig_metodo = False

        # Configuração do estado BLE
        self.status_BLE = mode_BLE.BLE_DISABLE
        self.status_connect_ble(self.status_BLE)
        self.ui.statusBLE.setEnabled(False)
        # Estado do BLE (habilita/desabilita o BLE)
        self.enable_BLE = False

        self.timer = QTimer()
        self.timer.timeout.connect(self.plot_chart)

        self.pen_chart = QPen()


    def scan_serial_ports(self):
        """
        Este método verifica e atualiza a lista de portas seriais disponíveis.

        - Verifica se o número de portas seriais disponíveis mudou.
        - Atualiza a lista de portas seriais disponíveis.
        - Limpa e preenche o ComboBox com os nomes das portas disponíveis.
        - Habilita o ComboBox e o botão de conexão se houver portas disponíveis, caso contrário, os desabilita.
        - Fecha a porta serial se estiver conectada, mas não estiver mais disponível.

        """
        # Verifica se o número de portas seriais disponíveis mudou
        if len(self.available_serial_ports) != len(QSerialPortInfo.availablePorts()):
            # Atualiza a lista de portas seriais disponíveis
            self.available_serial_ports = QSerialPortInfo.availablePorts()

            # Limpa e preenche o ComboBox com os nomes das portas disponíveis
            self.ui.serialPortsComboBox.clear()
            self.ui.serialPortsComboBox.addItems([x.portName() for x in self.available_serial_ports])

            # Habilita o ComboBox e o botão de conexão se houver portas disponíveis
            if not self.is_serial_port_connected:
                if self.ui.serialPortsComboBox.count() > 0:
                    self.ui.serialPortsComboBox.setEnabled(True)
                    self.ui.connectButton.setEnabled(True)
                else:
                    # Desabilita o ComboBox e o botão de conexão se não houver portas disponíveis
                    self.ui.serialPortsComboBox.setEnabled(False)
                    self.ui.connectButton.setEnabled(False)

        # Obtém os itens do ComboBox em uma lista
        combobox_items = [self.ui.serialPortsComboBox.itemText(i) for i in range(self.ui.serialPortsComboBox.count())]

        # Fecha a porta serial se estiver conectada, mas não estiver mais disponível
        if self.is_serial_port_connected and self.connected_serial_port.portName() not in combobox_items:
            print("Close" + self.connected_serial_port.portName())
            self.connected_serial_port.close()
            self.is_serial_port_connected = False
            self.ui.connectButton.setText("Connect")
            self.ui.connectButton.setEnabled(True)
            self.ui.serialPortsComboBox.setEnabled(True)
            self.ui.startButton.setEnabled(False)
        """
        Observação:
        - Este método verifica e atualiza as portas seriais disponíveis, mantendo o ComboBox e o botão de conexão atualizados.
        - Ele também fecha a porta serial se ela estiver conectada, mas não estiver mais disponível.
        """


    @Slot()
    def on_connect_button_clicked(self, checked):
        """
        Este método é chamado quando o botão de conexão é clicado.

        - Se a porta serial estiver conectada, desconecta-a, reabilita as opções e desativa o BLE (Bluetooth Low Energy).
        - Se a porta serial não estiver conectada, tenta conectá-la. Se bem-sucedido, configura a interface e habilita o BLE.

        :param checked: Indica se o botão de conexão foi verificado (clicado).
        """
        if self.is_serial_port_connected:
            # Se a porta estiver conectada, desconecta-a
            self.connected_serial_port.close()
            self.ui.connectButton.setText("Connect")
            self.ui.serialPortsComboBox.setEnabled(True)
            self.is_serial_port_connected = False
            self.ui.startButton.setEnabled(False)

            # Desativa o BLE (Bluetooth Low Energy) e interrompe o método em execução (se houver)
            self.status_connect_ble(mode_BLE.BLE_DISABLE)
            if self.runnig_metodo:
                self.on_stop_button_clicked()
        else:
            # Se a porta não estiver conectada, tenta conectá-la
            print(f"clicked connect_button {checked}")
            self.connected_serial_port.setPortName(self.ui.serialPortsComboBox.currentText())
            print(f"Connecting to {self.connected_serial_port.portName()}")

            # Abre a porta serial para leitura/escrita
            if self.connected_serial_port.open(QSerialPort.ReadWrite):
                print(f"Porta serial aberta com sucesso.")
                self.ui.connectButton.setText("Disconnect")
                self.ui.serialPortsComboBox.setEnabled(False)
                self.is_serial_port_connected = True
                self.ui.startButton.setEnabled(True)
                self.send_ble_check()
            else:
                # Exibe uma mensagem de erro se a porta não puder ser aberta
                self.is_serial_port_connected = False
                msgBox = QMessageBox()
                msgBox.setWindowTitle("COM Port Error!")
                msgBox.setIcon(QMessageBox.Warning)
                msgBox.setText("Selected COM port does not exist. Please verify the COM port Number.")
                msgBox.exec()
                return
        """
        Observação:
        - Este método gerencia a ação do botão de conexão, controlando a conexão e desconexão da porta serial, habilitando/desabilitando a interface e configurando a comunicação BLE.
        """


    @Slot()
    def read_serial_port_data(self):
        """
        Este método é chamado quando há dados disponíveis na porta serial.
        Ele lê os dados da porta serial, processa-os e toma ações com base no conteúdo.

        - Se receber 'disconnect_BLE', desconecta o dispositivo BLE, exibe uma mensagem de aviso e encerra a gravação de dados.
        - Se receber 'connect_BLE', conecta o dispositivo BLE.
        - Se receber 'uart_only', desabilita o BLE e mantém a comunicação apenas pela porta UART.

        Se o método atual for Cyclic Voltammetry (currentMedoto == 0):
        - Se receber 'Cycle;Voltage;Current', cria um arquivo e escreve o cabeçalho.
        - Se receber 'end_CV', fecha o arquivo.
        - Caso contrário, processa os dados, escreve no arquivo, atualiza o gráfico e armazena os dados na lista.

        Se o método atual for Amperometry (currentMedoto == 1):
        - Se receber 'Voltage(mV);Time(ms);Current', cria um arquivo e escreve o cabeçalho.
        - Se receber 'end_Amp', fecha o arquivo.
        - Caso contrário, processa os dados, escreve no arquivo, atualiza o gráfico e armazena os dados na lista.
        """
        if self.is_serial_port_connected:
            while self.connected_serial_port.canReadLine():
                linha = self.connected_serial_port.readLine().data().decode().strip()
                if linha != '\n':
                    print(f"Porta COM recebido: {linha}")
                    if linha == 'disconnect_BLE':
                        self.status_connect_ble(mode_BLE.BLE_DISCONNECT)
                        self.set_enable_BLE(True)
                        if self.runnig_metodo:
                            msgBox = QMessageBox()
                            msgBox.setWindowTitle("BLE Status!")
                            msgBox.setIcon(QMessageBox.Warning)
                            msgBox.setText("Device disconnected by bluetooth.")
                            msgBox.exec()
                            self.close_file()
                            self.enabled_buttons()
                            list_decoded_data.clear()
                            self.timer.stop()
                            self.timer.deleteLater()
                            QCoreApplication.removePostedEvents(self.timer)
                            self.ui.stopButton.setEnabled(False)
                    elif linha == 'connect_BLE':
                        self.status_connect_ble(mode_BLE.BLE_CONNECT)
                        self.set_enable_BLE(True)
                    elif linha == 'uart_only':
                        self.status_connect_ble(mode_BLE.BLE_DISABLE)
                        self.set_enable_BLE(False)
                    elif self.currentMedoto == 0:
                        linha_decoded = linha.replace("\t", ";")
                        if linha_decoded == 'Cycle;Voltage;Current':
                            self.create_file()
                            self.write_file(linha_decoded)
                        elif linha_decoded == 'end_CV':
                            self.close_file()
                        else:
                            linha_decoded_f = linha_decoded.split(";")
                            if len(linha_decoded_f) >= 2:
                                self.write_file(linha_decoded)
                                dado = [float(linha_decoded_f[1]), float(linha_decoded_f[2])]
                                list_decoded_data.append(dado)
                                self.timer.start(1)
                    elif self.currentMedoto == 1:
                        linha_decoded = linha.replace("\t", ";")
                        if 'Voltage(mV);Time(ms);Current' in linha_decoded:
                            self.create_file()
                            self.write_file(linha_decoded)
                        elif linha_decoded == 'end_Amp':
                            self.close_file()
                        else:
                            linha_decoded_f = linha_decoded.split(";")
                            if len(linha_decoded_f) >= 2:
                                self.write_file(linha_decoded)
                                dado = [float(linha_decoded_f[1]), float(linha_decoded_f[2])]
                                list_decoded_data.append(dado)
                                self.timer.start(1)
        """
        Observação:
        - Este método é responsável por processar os dados recebidos pela porta serial e controlar as ações com base nos comandos recebidos.
        - Os comandos 'disconnect_BLE', 'connect_BLE' e 'uart_only' são usados para controlar a comunicação com dispositivos BLE.
        - Dependendo do método atual (Cyclic Voltammetry ou Amperometry), os dados são registrados em arquivos e atualizações são feitas no gráfico.
        """

    @Slot()
    def on_clear_button_clicked(self):
        """
        Limpa o gráfico personalizado quando o botão 'Clear' é clicado.

        Isso remove todos os dados e curvas do gráfico personalizado.

        :return: Nenhum retorno.
        """
        self.custom_plot.clearPlottables()
        self.custom_plot.removePlottable(0)
        if self.runnig_metodo:
            self.curve = QCPCurve(self.custom_plot.xAxis, self.custom_plot.yAxis)
            self.curve.setPen(self.pen_chart)
        self.custom_plot.replot()


    @Slot()
    def page_changed(self, index):
        """
        Manipula a mudança da página atual no QToolBox.

        Esta função é chamada quando a página atual do QToolBox é alterada.
        Ela atualiza o estado interno `currentMedoto` com o índice da página atual
        e configura as etiquetas dos eixos do gráfico personalizado de acordo com a página atual.

        :param index: O índice da página atual no QToolBox.
        :type index: int
        :return: Nenhum retorno.
        """
        print("Current page changed to:", index)
        self.currentMedoto = index
        if index == 0:
            print("CVolt:", index)
            self.custom_plot.xAxis.setLabel("Voltage (mV)")
            self.custom_plot.yAxis.setLabel("Current (uA)")
        else:
            print("Amper:", index)
            self.custom_plot.xAxis.setLabel("Time (mSec)")
            self.custom_plot.yAxis.setLabel("Current (uA)")

        self.custom_plot.replot()


    @Slot()
    def on_start_button_clicked(self, checked):
        """
        Manipula o clique no botão de início de medição.

        Esta função é chamada quando o botão de início é clicado. Ela executa várias verificações,
        como a existência do arquivo de saída, a conexão da porta serial e o estado do Bluetooth,
        para determinar se a medição pode ser iniciada. Se todas as verificações passarem, a medição
        é iniciada e os botões são desabilitados.

        :param checked: Indica se o botão está marcado (True) ou desmarcado (False).
        :type checked: bool
        :return: Nenhum retorno.
        """
        self.on_clear_button_clicked()
        # Adiciona uma curva ao gráfico
        self.curve = QCPCurve(self.custom_plot.xAxis, self.custom_plot.yAxis)

        print("Start button clicked -> ", checked)  # Pode tirar o check?
        inicia_medida = True

        if os.path.exists(self.csv_file_name):
            if os.path.isfile(self.csv_file_name):
                # Código a ser executado se o arquivo existir
                # Arquivo existe
                # Exibir QMessageBox para confirmar continuação
                resposta = QMessageBox.question(
                    None,
                    "Arquivo Existente",
                    "O arquivo já existe. Deseja continuar mesmo assim?",
                    QMessageBox.Yes | QMessageBox.No
                )
                if resposta == QMessageBox.Yes:
                    # Código a ser executado se o usuário escolher continuar
                    inicia_medida = True
                else:
                    # Código a ser executado se o usuário escolher não continuar
                    inicia_medida = False
            else:
                # Código a ser executado se existir um diretório com o mesmo nome
                QMessageBox.about(self, "Start Button", "Could not start, folder output file does not exist")
        elif not (self.csv_file_name.endswith('.csv') or self.csv_file_name.endswith('.txt')):
            # Arquivo não existe
            inicia_medida = False
            QMessageBox.about(self, "Start Button", "Could not start, output file does not exist")

        elif not self.is_serial_port_connected:
            inicia_medida = False
            QMessageBox.about(self, "Serial Port", "Serial port not connected")

        elif self.enable_BLE == True and self.status_BLE == mode_BLE.BLE_DISCONNECT:
            inicia_medida = False
            QMessageBox.about(self, "Bluetooth Status", "Device disconnected by bluetooth. Connect to continue")

        if inicia_medida == True:
            self.ui.stopButton.setEnabled(True)
            self.disable_buttons()
            self.iniciaMetodo()


    @Slot()
    def on_stop_button_clicked(self):
        """
        Manipula o clique no botão de parada de medição.

        Esta função é chamada quando o botão de parada é clicado. Ela envia um comando "stop" para a porta serial
        para interromper a medição, fecha o arquivo de saída, habilita os botões e limpa os dados da lista.

        :return: Nenhum retorno.
        """
        self.connected_serial_port.write("stop\n".encode())
        self.close_file()
        self.enabled_buttons()
        list_decoded_data.clear()
        self.timer.stop()
        self.timer.deleteLater()
        QCoreApplication.removePostedEvents(self.timer)
        self.ui.stopButton.setEnabled(False)


    @Slot()
    def on_browse_button_clicked(self):
        """
        Abre um navegador de arquivos e permite selecionar um diretório onde o arquivo de saída CSV será salvo.

        Esta função abre uma janela de diálogo de arquivo para permitir ao usuário escolher o local e nome do arquivo de saída.
        O nome do arquivo selecionado é exibido no campo de texto da interface gráfica.

        :return: Nenhum retorno.
        """
        file_name, _ = QFileDialog.getSaveFileName(self, "Salvar arquivo", "", "Arquivo TXT (*.txt);;Arquivo CSV (*.csv)")
        if file_name:
            self.csv_file_name = file_name
            self.ui.outputFileName.setText(file_name)

    def iniciaMetodo(self):
        """
        Inicia a execução do método selecionado.

        Esta função prepara e envia os parâmetros do método selecionado para a porta serial.
        O método pode ser Voltametria Cíclica (CV) ou Amperometria (Amp), dependendo da escolha do usuário.
        Os parâmetros são configurados com base nas configurações definidas pelo usuário na interface gráfica.

        :return: Nenhum retorno.
        """
        send_metodo = " "
        set_to_zero = "0"
        if self.currentMedoto == 0:
            # Configuração para Voltametria Cíclica (CV)
            if self.cyclic_voltammetry.get_set_to_zero():
                set_to_zero = "0"
            else:
                set_to_zero = "1"

            send_metodo = "0;" + str(self.cyclic_voltammetry.get_lmp_gain()) + ";" + str(self.cyclic_voltammetry.get_cycles()) + ";" + str(self.cyclic_voltammetry.get_start_voltage())
            send_metodo = send_metodo + ";" + str(self.cyclic_voltammetry.get_end_voltage()) + ";" + str(self.cyclic_voltammetry.get_vertex_1()) + ";" + str(self.cyclic_voltammetry.get_vertex_2())
            send_metodo = send_metodo + ";" + str(self.cyclic_voltammetry.get_step_voltage()) + ";" + str(self.cyclic_voltammetry.get_scan_rate()) + ";" + set_to_zero

            self.connected_serial_port.write(send_metodo.encode())
            self.ui.statusbar.showMessage("Executando Voltametria Ciclica ...")
            self.pen_chart = QPen(QColor(80, 80, 255))
            self.pen_chart.setWidth(4)
            self.curve.setPen(self.pen_chart)
            self.custom_plot.xAxis.setRange(float(self.cyclic_voltammetry.get_vertex_1())*1.2, float(self.cyclic_voltammetry.get_vertex_2())*1.2)
        else:
            # Configuração para Amperometria (Amp)
            if self.amperometry.get_set_to_zero():
                set_to_zero = "0"
            else:
                set_to_zero = "1"

            send_metodo = "1;" + str(self.amperometry.get_lmp_gain()) + ";" + str(self.amperometry.get_pre_stepV()) + ";" + str(self.amperometry.get_quietTime())
            send_metodo = send_metodo + ";" + str(self.amperometry.get_v1()) + ";" + str(self.amperometry.get_t1()) + ";" + str(self.amperometry.get_v2())
            send_metodo = send_metodo + ";" + str(self.amperometry.get_t2()) + ";" + str(self.amperometry.get_samples()) + ";"
            send_metodo = send_metodo + str(self.amperometry.get_ranges()) + ";" + set_to_zero

            self.connected_serial_port.write(send_metodo.encode())
            self.ui.statusbar.showMessage("Executando Amperometria ...")
            self.pen_chart = QPen(QColor(40, 40, 255))
            self.pen_chart.setWidth(4)
            self.curve.setPen(self.pen_chart)


    def update_plot_chart(self, x_data, y_data):
        """
        Atualiza o gráfico com novos dados.

        Esta função adiciona novos pontos de dados (x_data, y_data) ao gráfico, atualiza os eixos conforme necessário e
        solicita a atualização do gráfico.

        :param x_data: Dados para o eixo X do gráfico.
        :param y_data: Dados para o eixo Y do gráfico.
        :return: Nenhum retorno.
        """
        self.curve.addData(x_data, y_data)
        self.custom_plot.yAxis.rescale()
        if self.currentMedoto != 0:
            self.custom_plot.xAxis.rescale()
        self.custom_plot.replot()

    def plot_chart(self):
        """
        Plots data on the chart.

        This function plots the data stored in `list_decoded_data` on the chart, updating the chart with the most
        recent data. It removes the oldest data point from the list and adjusts the axes as necessary. After adding the
        new data, the function calls `replot` to update the chart display.

        If there is no data in `list_decoded_data` and the method is not running, it enables the buttons, stops the
        timer, and updates the "Start" button text.

        :return: No return value.
        """
        if list_decoded_data:
            ultimo_dado = list_decoded_data.pop(0)
            self.curve.addData(ultimo_dado[0], ultimo_dado[1])
            self.custom_plot.yAxis.rescale()
            if self.currentMedoto != 0:
                self.custom_plot.xAxis.rescale()
            self.custom_plot.replot()
        elif self.runnig_metodo == False:
            self.enabled_buttons()
            self.timer.stop()
            self.timer.deleteLater()
            QCoreApplication.removePostedEvents(self.timer)
            self.ui.startButton.setText("Start")

    def enabled_buttons(self):
        """
        Enable various UI buttons.

        This function enables the "Start," "Connect," "Browse," and "Output File Name" elements in the user interface.

        :return: No return value.
        """

        self.ui.startButton.setEnabled(True)
        self.ui.connectButton.setEnabled(True)
        self.ui.toolBox_metodo.setEnabled(True)
        self.ui.browseButton.setEnabled(True)
        self.ui.outputFileName.setEnabled(True)

    def disable_buttons(self):
        """
        Disable various UI buttons.

        This function disables the "Start," "Connect," "Browse," and "Output File Name" elements in the user interface.

        :return: No return value.
        """

        self.ui.startButton.setEnabled(False)
        self.ui.connectButton.setEnabled(False)
        self.ui.toolBox_metodo.setEnabled(False)
        self.ui.browseButton.setEnabled(False)
        self.ui.outputFileName.setEnabled(False)

    def connect_ui_signals(self):
        """
        Connect UI signals to their corresponding slots.

        This function connects various UI buttons and elements to their corresponding slots (functions) in the application.

        :return: No return value.
        """

        self.ui.connectButton.clicked.connect(self.on_connect_button_clicked)
        self.ui.startButton.clicked.connect(self.on_start_button_clicked)
        self.ui.toolBox_metodo.currentChanged.connect(self.page_changed)
        self.ui.clearButton.clicked.connect(self.on_clear_button_clicked)
        self.ui.browseButton.clicked.connect(self.on_browse_button_clicked)
        self.ui.stopButton.clicked.connect(self.on_stop_button_clicked)

    def calc_delayTime(self):
        """
        Calculate and display the delay time based on step voltage and scan rate.

        This function calculates the delay time (in milliseconds) based on the step voltage and scan rate values
        entered by the user and displays it in the UI.

        :return: No return value.
        """
        self.ui.CV_delayTimeValue.setText(f" {int((1000.0 * self.ui.CV_stepVoltageValue.value())/self.ui.CV_scanRateValue.value())}")

    def connect_inicialCV(self):
        """
        Connects initial configuration settings for Cyclic Voltammetry (CV).

        This function connects various UI elements to corresponding setters in the CyclicVoltammetry object
        and initializes the initial CV settings based on the values in the UI.

        :return: No return value.
        """
        # Connect UI elements to CV setters
        self.ui.CV_lmpGainValue.currentTextChanged.connect(self.cyclic_voltammetry.set_lmp_gain)
        self.ui.CV_cyclesValue.valueChanged.connect(self.cyclic_voltammetry.set_cycles)
        self.ui.CV_startVoltageValue.valueChanged.connect(self.cyclic_voltammetry.set_start_voltage)
        self.ui.CV_endVoltageValue.valueChanged.connect(self.cyclic_voltammetry.set_end_voltage)
        self.ui.CV_vertex1Value.valueChanged.connect(self.cyclic_voltammetry.set_vertex_1)
        self.ui.CV_vertex2Value.valueChanged.connect(self.cyclic_voltammetry.set_vertex_2)
        self.ui.CV_stepVoltageValue.valueChanged.connect(self.cyclic_voltammetry.set_step_voltage)
        self.ui.CV_scanRateValue.valueChanged.connect(self.cyclic_voltammetry.set_scan_rate)
        self.ui.setToZeroValue.stateChanged.connect(self.cyclic_voltammetry.set_set_to_zero)

        # Connect CV settings to delay time calculation
        self.ui.CV_stepVoltageValue.valueChanged.connect(self.calc_delayTime)
        self.ui.CV_scanRateValue.valueChanged.connect(self.calc_delayTime)
        self.calc_delayTime()

        # Initialize CV settings
        self.cyclic_voltammetry.set_lmp_gain(self.ui.CV_lmpGainValue.currentText())
        self.cyclic_voltammetry.set_cycles(self.ui.CV_cyclesValue.value())
        self.cyclic_voltammetry.set_start_voltage(self.ui.CV_startVoltageValue.value())
        self.cyclic_voltammetry.set_end_voltage(self.ui.CV_endVoltageValue.value())
        self.cyclic_voltammetry.set_vertex_1(self.ui.CV_vertex1Value.value())
        self.cyclic_voltammetry.set_vertex_2(self.ui.CV_vertex2Value.value())
        self.cyclic_voltammetry.set_step_voltage(self.ui.CV_stepVoltageValue.value())
        self.cyclic_voltammetry.set_scan_rate(self.ui.CV_scanRateValue.value())

    def connect_inicialAmp(self):
        """
        Connects initial configuration settings for Amperometry.

        This function connects various UI elements to corresponding setters in the Amperometry object
        and initializes the initial Amperometry settings based on the values in the UI.

        :return: No return value.
        """
        # Connect UI elements to Amperometry setters
        self.ui.Amp_lmpGainValue.currentTextChanged.connect(self.amperometry.set_lmp_gain)
        self.ui.Amp_startVoltageValue.valueChanged.connect(self.amperometry.set_pre_stepV)
        self.ui.Amp_quietTimeValue.valueChanged.connect(self.amperometry.set_quietTime)
        self.ui.Amp_FirstStepEValue.valueChanged.connect(self.amperometry.set_v1)
        self.ui.Amp_FirstStepTimeValue.valueChanged.connect(self.amperometry.set_t1)
        self.ui.Amp_SecondStepEValue.valueChanged.connect(self.amperometry.set_v2)
        self.ui.Amp_SecondStepTimeValue.valueChanged.connect(self.amperometry.set_t2)
        self.ui.Amp_SampleValue.valueChanged.connect(self.amperometry.set_samples)
        self.ui.Amp_LmpRangeValue.currentIndexChanged.connect(self.amperometry.set_ranges)
        self.ui.setToZeroValue.stateChanged.connect(self.amperometry.set_set_to_zero)

        # Initialize Amperometry settings
        self.amperometry.set_lmp_gain(self.ui.Amp_lmpGainValue.currentText())
        self.amperometry.set_pre_stepV(self.ui.Amp_startVoltageValue.value())
        self.amperometry.set_quietTime(self.ui.Amp_quietTimeValue.value())
        self.amperometry.set_v1(self.ui.Amp_FirstStepEValue.value())
        self.amperometry.set_t1(self.ui.Amp_FirstStepTimeValue.value())
        self.amperometry.set_v2(self.ui.Amp_SecondStepEValue.value())
        self.amperometry.set_t2(self.ui.Amp_SecondStepTimeValue.value())
        self.amperometry.set_samples(self.ui.Amp_SampleValue.value())
        self.amperometry.set_ranges(self.ui.Amp_LmpRangeValue.currentIndex())

    def load_ui(self):
        """
        Load the user interface from the .ui file and perform additional setup.

        This function loads the user interface defined in the "MainWindows.ui" file using the QUiLoader, and
        performs additional setup, such as setting up the chart, loading images for labels, and setting the window icon.

        :return: No return value.
        """
        loader = QUiLoader()
        path = os.path.join(os.path.dirname(__file__), "forms/MainWindows.ui")
        ui_file = QFile(path)
        ui_file.open(QFile.ReadOnly)
        self.ui = loader.load(ui_file, self)
        ui_file.close()

        self.setup_chart()

        self.ui.label_fapesp.setPixmap(QPixmap('./resources/logos/logos-fapesp2.png'))
        self.ui.label_iq.setPixmap(QPixmap('./resources/logos/logo-iqusp2.png'))
        self.ui.label_poli.setPixmap(QPixmap('./resources/logos/poli-usp2.png'))
        self.setWindowIcon(QIcon('./resources/logos/CV_icon.png'))

    def setup_chart(self):
        """
        Set up the chart widget with custom styling.

        This function configures the chart widget with a custom appearance, including background color, axis labels,
        and colors.

        :return: No return value.
        """
        self.ui.chartWidget.setStyleSheet("background-color: transparent;")
        self.custom_plot = QCustomPlot(self.ui.chartWidget)
        self.custom_plot.setBackground(QColor(51, 56, 70))
        self.custom_plot.xAxis.setLabel("Voltage (mV)")
        self.custom_plot.xAxis.setBasePen(QPen(QColor(255, 255, 255)))
        self.custom_plot.xAxis.setBasePen(QPen(QColor(255, 255, 255)))
        self.custom_plot.xAxis.setTickPen(QPen(QColor(255, 255, 255)))
        self.custom_plot.xAxis.setTickLabelColor(QColor(255, 255, 255))
        self.custom_plot.xAxis.setLabelColor(QColor(255, 255, 255))
        self.custom_plot.yAxis.setLabel("Current (uA)")
        self.custom_plot.yAxis.setBasePen(QPen(QColor(255, 255, 255)))
        self.custom_plot.yAxis.setBasePen(QPen(QColor(255, 255, 255)))
        self.custom_plot.yAxis.setTickPen(QPen(QColor(255, 255, 255)))
        self.custom_plot.yAxis.setTickLabelColor(QColor(255, 255, 255))
        self.custom_plot.yAxis.setLabelColor(QColor(255, 255, 255))

        self.custom_plot.clearGraphs()
        self.custom_plot.addGraph()

        self.custom_plot.setInteractions(QCP.iRangeDrag | QCP.iRangeZoom | QCP.iSelectPlottables)

    def resizeEvent(self,event):
        """
        Handle the resize event of the main window.

        This function resizes the chart widget to fit the window's size when the window is resized.

        :param event: The resize event.
        :type event: QResizeEvent
        :return: No return value.
        """
        self.custom_plot.setFixedWidth(self.ui.chartWidget.width())
        self.custom_plot.setFixedHeight(self.ui.chartWidget.height())
        QMainWindow.resizeEvent(self, event)

    def create_file(self):
        """
        Create and open a CSV file for data logging.

        This function creates and opens a CSV file with the specified file name for data logging. The file is opened
        in write mode.

        :return: No return value.
        """
        print("open csv file...")
        self.csv_file = open(self.csv_file_name, "w", newline='')
        self.runnig_metodo = True

    def close_file(self):
        """
        Close the CSV file.

        This function closes the previously opened CSV file for data logging.

        :return: No return value.
        """
        print("closing csv file...")
        self.csv_file.close()
        self.csv_file = None
        self.runnig_metodo = False

    def write_file(self, data):
        """
        Write data to the CSV file.

        This function writes the specified data to the opened CSV file. The data is written as a line in the CSV file.

        :param data: The data to be written to the CSV file.
        :type data: str
        :return: No return value.
        """
        self.csv_file.write(data+'\n')

    def status_connect_ble(self, status):
        """
        Set the status of BLE (Bluetooth Low Energy) connection and update the UI accordingly.

        This function sets the status of BLE connection, which can be one of the following modes:
        - BLE_CONNECT: Indicates that BLE is connected.
        - BLE_DISCONNECT: Indicates that BLE is disconnected.
        - BLE_DISABLE: Indicates that BLE is not in use.

        The function updates the UI to reflect the current BLE status by changing the text and appearance of the
        Bluetooth status indicator.

        :param status: The status of BLE connection (BLE_CONNECT, BLE_DISCONNECT, or BLE_DISABLE).
        :type status: int
        :return: No return value.
        """
        self.status_BLE = status
        if status == mode_BLE.BLE_CONNECT:
            self.ui.statusBLE.setText("Connected")
            self.ui.statusBLE.setStyleSheet("""
            QCheckBox::indicator {
                width: 12px;
                height: 12px;
                border-radius: 6px;
                border-style: solid;
                border-width: 1px;
                border-color: white;
                background-color: qradialgradient(spread:pad,
                                            cx:0.5,
                                            cy:0.5,
                                            radius:0.9,
                                            fx:0.5,
                                            fy:0.5,
                                            stop:0 rgba(0, 255, 0, 255),
                                            stop:1 rgba(0, 64, 0, 255));
            }
            QCheckBox{
                background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0.268657 rgba(36, 39, 50, 255), stop:1 rgba(51, 255, 70, 255));
                border-radius: 5px;
            }
            """)
        elif status == mode_BLE.BLE_DISCONNECT:
            self.ui.statusBLE.setText("Disconnected")
            self.ui.statusBLE.setStyleSheet("""
            QCheckBox::indicator {
                width: 12px;
                height: 12px;
                border-radius: 6px;
                border-style: solid;
                border-width: 1px;
                border-color: white;
                background-color: qradialgradient(spread:pad,
                                        cx:0.5,
                                        cy:0.5,
                                        radius:0.9,
                                        fx:0.5,
                                        fy:0.5,
                                        stop:0 rgba(255, 0, 0, 255),
                                        stop:1 rgba(64, 0, 0, 255));
            }
            QCheckBox{
                background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0.268657 rgba(36, 39, 50, 255), stop:1 rgba(255, 56, 70, 255));
                border-radius: 5px;
            }
            """)
        elif status == mode_BLE.BLE_DISABLE:
            self.ui.statusBLE.setText("Not use")
            self.ui.statusBLE.setStyleSheet("""
            QCheckBox::indicator {
                width: 16px;
                height: 16px;
                background-color: rgba(150, 150, 150, 255);
                border-radius: 8px;
                border-style: solid;
                border-width: 1px;
                border-color: white;
            }
            QCheckBox{
                background-color: rgba(200, 200, 200, 255);
                color: rgba(100, 100, 100, 255);
            }
            """)



    def send_ble_check(self):
        """
        Send a command to the connected serial port to check the status of BLE (Bluetooth Low Energy).

        This function sends a command to the connected serial port to request the status of BLE. It can be used to
        check if the Bluetooth device is connected or disconnected.

        :return: No return value.
        """
        self.connected_serial_port.write("status_BLE\n".encode())

    def set_enable_BLE(self, state):
        """
        Set the state of enabling or disabling BLE (Bluetooth Low Energy) communication.

        This function sets the state of enabling or disabling BLE communication based on the provided `state`. When BLE
        is enabled, the device can communicate via Bluetooth. When disabled, it won't use Bluetooth communication.

        :param state: The state of enabling or disabling BLE (True to enable, False to disable).
        :type state: bool
        :return: No return value.
        """
        self.enable_BLE = state

if __name__ == "__main__":
    """
    The main entry point for the Potentiostat application.

    This block of code initializes the PyQt application, sets custom fonts, creates the main window of the application,
    and enters the application event loop.

    :return: No return value.
    """
    app = QApplication(sys.argv)

    QFontDatabase.addApplicationFont("./resources/fonts/AvenirNextLTPro-Regular.otf")
    QFontDatabase.addApplicationFont("./resources/fonts/AvenirNextLTPro-Bold.otf")

    widget = MainWindow()
#    widget.setFixedWidth(1090)
#    widget.setFixedHeight(600)

    # Set window title
    widget.setWindowTitle("Potentiostat")

    widget.show()
    sys.exit(app.exec_())

