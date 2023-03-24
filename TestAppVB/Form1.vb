Imports LennoxChiller

Public Class Form1
    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        Dim Input As String
        Dim output As String


        Input = "{""MODELID"": 1,""SUPPLIERID"": 1, ""airflow"": 3600, ""fantypeoption"": 10, ""totaloptionpressuredrop"": 100}"
        Dim fan As New LennoxChiller.CalculateFan()
        output = fan.GetFanPerformance(Input)
        MsgBox(output)

    End Sub
End Class
