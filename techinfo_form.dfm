object TechinfoForm: TTechinfoForm
  Left = 267
  Top = 187
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Technical Information'
  ClientHeight = 325
  ClientWidth = 454
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object infos: TRichEdit
    Left = 0
    Top = 0
    Width = 454
    Height = 325
    Align = alClient
    ReadOnly = True
    TabOrder = 1
    WordWrap = False
  end
  object bOk: TButton
    Left = 366
    Top = 288
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
end
