﻿Imports LennoxRooftop

Public Class Form1
    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        Dim Input As String
        Dim output As String
        Dim init As Boolean

        Input = "{""MODELID"": ""B6FHC025SP1M"",""SUPPLIERID"": 1, ""airflow"": 1, ""fantypeoption"": 10, ""totaloptionpressuredrop"": 200, ""density"": 1.2, ""temperature"": 20}"
        Dim chiller As New LennoxRooftop.Rooftop()
        init = chiller.Init()
        If init = True Then
            output = chiller.GetFanPerformance(Input)
            MsgBox(output)
        End If
    End Sub
End Class
