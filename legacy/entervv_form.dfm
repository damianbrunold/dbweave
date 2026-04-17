object EnterVVForm: TEnterVVForm
  Left = 230
  Top = 204
  BorderStyle = bsDialog
  Caption = 'Verstärkungsverhältnis'
  ClientHeight = 98
  ClientWidth = 219
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object labVV: TLabel
    Left = 8
    Top = 8
    Width = 200
    Height = 13
    Caption = '&Geben Sie das Verstärkungsverhältnis ein:'
    FocusControl = VV1
  end
  object labVV12: TLabel
    Left = 33
    Top = 24
    Width = 6
    Height = 13
    Caption = '_'
  end
  object labVV23: TLabel
    Left = 70
    Top = 24
    Width = 6
    Height = 13
    Caption = '_'
  end
  object labVV34: TLabel
    Left = 106
    Top = 24
    Width = 6
    Height = 13
    Caption = '_'
  end
  object labVV45: TLabel
    Left = 140
    Top = 24
    Width = 6
    Height = 13
    Caption = '_'
  end
  object labVV56: TLabel
    Left = 178
    Top = 24
    Width = 6
    Height = 13
    Caption = '_'
  end
  object VV1: TEdit
    Left = 8
    Top = 24
    Width = 22
    Height = 21
    TabOrder = 0
    Text = '1'
    OnChange = VV1Change
  end
  object VV2: TEdit
    Left = 43
    Top = 24
    Width = 22
    Height = 21
    TabOrder = 1
    Text = '1'
    OnChange = VV2Change
  end
  object VV3: TEdit
    Left = 80
    Top = 24
    Width = 22
    Height = 21
    TabOrder = 2
    OnChange = VV3Change
  end
  object VV4: TEdit
    Left = 114
    Top = 24
    Width = 22
    Height = 21
    Enabled = False
    TabOrder = 3
    OnChange = VV4Change
  end
  object VV5: TEdit
    Left = 150
    Top = 24
    Width = 22
    Height = 21
    Enabled = False
    TabOrder = 4
    OnChange = VV5Change
  end
  object VV6: TEdit
    Left = 190
    Top = 24
    Width = 22
    Height = 21
    Enabled = False
    TabOrder = 5
    OnChange = VV6Change
  end
  object bOk: TButton
    Left = 24
    Top = 64
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 6
    OnClick = bOkClick
  end
  object bCancel: TButton
    Left = 120
    Top = 64
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Abbrechen'
    ModalResult = 2
    TabOrder = 7
  end
end
