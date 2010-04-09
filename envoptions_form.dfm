object EnvironmentOptionsForm: TEnvironmentOptionsForm
  Left = 259
  Top = 166
  BorderStyle = bsDialog
  Caption = 'Environment options'
  ClientHeight = 134
  ClientWidth = 202
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object lLanguage: TLabel
    Left = 8
    Top = 10
    Width = 51
    Height = 13
    Caption = '&Language:'
    FocusControl = cbLanguage
  end
  object cbLanguage: TComboBox
    Left = 72
    Top = 8
    Width = 121
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 0
    Items.Strings = (
      'English'
      'German')
  end
  object bOk: TButton
    Left = 16
    Top = 96
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
    OnClick = bOkClick
  end
  object bCancel: TButton
    Left = 112
    Top = 96
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object ShowSplash: TCheckBox
    Left = 8
    Top = 40
    Width = 185
    Height = 17
    Caption = '&Display splashscreen'
    TabOrder = 3
  end
end
