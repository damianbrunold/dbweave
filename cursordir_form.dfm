object CursorDirForm: TCursorDirForm
  Left = 241
  Top = 179
  BorderStyle = bsDialog
  Caption = 'Cursor movement'
  ClientHeight = 142
  ClientWidth = 257
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnMouseUp = FormMouseUp
  PixelsPerInch = 96
  TextHeight = 13
  object cdUp: TSpeedButton
    Left = 104
    Top = 16
    Width = 41
    Height = 24
    AllowAllUp = True
    GroupIndex = 1
    Caption = '&Up'
    Flat = True
    OnClick = cdUpClick
  end
  object cdLeft: TSpeedButton
    Left = 64
    Top = 40
    Width = 39
    Height = 24
    AllowAllUp = True
    GroupIndex = 2
    Caption = '&Left'
    Flat = True
    OnClick = cdLeftClick
  end
  object cdRight: TSpeedButton
    Left = 144
    Top = 40
    Width = 41
    Height = 24
    AllowAllUp = True
    GroupIndex = 2
    Caption = '&Right'
    Flat = True
    OnClick = cdRightClick
  end
  object cdDown: TSpeedButton
    Left = 104
    Top = 64
    Width = 41
    Height = 24
    AllowAllUp = True
    GroupIndex = 1
    Caption = '&Down'
    Flat = True
    OnClick = cdDownClick
  end
  object bOk: TButton
    Left = 40
    Top = 104
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
    OnClick = bOkClick
  end
  object bCancel: TButton
    Left = 144
    Top = 104
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
    OnClick = bCancelClick
  end
end
