<h1 align="center">Potenciostato Software - Controle e Aquisição de Dados</h1>

Este software foi desenvolvido como parte do [Projeto Potenciostato](https://github.com/Eletroquest/Potenciostato/tree/main/Firmware) e tem como objetivo controlar e coletar dados de potenciostato, salvando-os em formatos CSV/Txt para análise posterior.

# Tecnologias Utilizadas
- Qt 5.15.2: A interface gráfica do software foi construída usando o framework Qt, com ênfase na versão 5.15.2.
- Python 3.11.4: A lógica do programa foi implementada em Python, na versão 3.11.4.<img loading="lazy" src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/python/python-original-wordmark.svg" width="30" height="30"/>
- PyQt: Para a integração do Qt com Python, utilizamos a biblioteca PyQt.<img loading="qt" src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/qt/qt-original.svg" width="30" height="30"/>

# Configuração de Ambiente e Dependências
## Instalação das bibliotecas
Antes de iniciar o desenvolvimento, certifique-se de ter as seguintes bibliotecas instaladas:
- Python 3.11.4: Utilizado como linguagem principal para o desenvolvimento.
- PySide2 5.15.2.1: A biblioteca PySide2 é a principal dependência para a interface gráfica.

## Opcional: Miniconda para Gerenciamento de Ambiente
Para facilitar o gerenciamento do ambiente e das dependências do projeto, recomendamos o uso do Miniconda. Siga os passos abaixo:
- Baixe o Miniconda no [site oficial](https://docs.conda.io/en/latest/miniconda.html) (versão: conda 4.13.0).

- Incialize um `virtualenv`:
```bash
$ pip install virtualenv
$ virutalenv venv
```

- Com o ambiente virtual criado, instale as dependências do projeto a partir do arquivo requirements.txt:
```bash
$ pip install -r requirements.txt
```

## Utilizando o Qt Creator

Para trabalhar com o Qt Creator, siga as instruções abaixo:

- Abra o projeto no Qt Creator utilizando o arquivo main.pyproject.

- No menu Projects, selecione a versão de Python correspondente ao ambiente virtual criado anteriormente como Interpreter.


# Executando o projeto

Para executar o projeto, basta utilizar o comando a seguir no terminal ou utilizar a opção `Run` no Qt Creator (após configurado corretamente).

- Comando terminal
```bash
$ python mainwindow.py
```

# Descrição
Esta aplicação oferece uma solução eficiente e fácil de usar para controle e aquisição de dados do seu potenciostato. Desenvolvida com tecnologias modernas, proporciona uma interface amigável para facilitar o seu trabalho de pesquisa ou experimentação. Aproveite ao máximo as funcionalidades do Projeto Potenciostato com este software de controle e aquisição de dados.

