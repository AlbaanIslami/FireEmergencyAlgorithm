package View;

import Controller.Controller;

import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.border.CompoundBorder;
import java.awt.*;

/**
 * @author Nezar, Alban on 2022-12-20.
 * @project Project_group_2
 */
public class PanelNode7_Node8 extends JPanel {

    private JList<String> node7;
    private JList<String> node8;
    private Controller controller;
    public PanelNode7_Node8(Controller controller, int width, int height, int margin)
    {
        this.controller = controller;

        Border border = this.getBorder();
        Border emptyBorder = BorderFactory.createEmptyBorder(margin, margin, margin, margin);
        setBorder(new CompoundBorder(border, emptyBorder));
        //data has type Object[]

        Font font = new Font("New Times Roman", Font.PLAIN, 14);
        setBorder(BorderFactory.createTitledBorder("                                  NODE 7                                                                                NODE 8                                   "));

        node7 = new JList<>();
        node8 = new JList<>();

        node7.setFont(font);
        node8.setFont(font);

        JScrollPane s = new JScrollPane(node7);
        JScrollPane s1 = new JScrollPane(node8);

        s.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        s.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        s.setPreferredSize(new Dimension(350-margin, height-2*margin));
        s1.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        s1.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        s1.setPreferredSize(new Dimension(350-margin, height-2*margin));

        node7.setSelectionMode(DefaultListSelectionModel.SINGLE_SELECTION);
        node8.setSelectionMode(DefaultListSelectionModel.SINGLE_SELECTION);
        add(s,BorderLayout.WEST);
        add(s1,BorderLayout.CENTER);
    }

    public void updateNode7(String[] node7_info) {
        node7.setListData(node7_info);
    }

    public void updateNode8(String[] node8_info) {
        node8.setListData(node8_info);
    }
}
