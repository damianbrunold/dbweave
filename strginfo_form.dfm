object StrgInfoForm: TStrgInfoForm
  Left = 570
  Top = 269
  Width = 351
  Height = 346
  BorderIcons = []
  Caption = 'Webstuhl Informationen'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object infos: TRichEdit
    Left = 0
    Top = 0
    Width = 335
    Height = 310
    TabStop = False
    Align = alClient
    ReadOnly = True
    TabOrder = 0
  end
  object bOk: TButton
    Left = 134
    Top = 280
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
end
