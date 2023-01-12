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
public class MainPanel extends JPanel {
    private Controller controller;
    private int width;
    private int height;
    BorderLayout layout;
    PanelNorth panelNorth;
    PanelSouth panelSouth;
    public MainPanel (Controller controller, int width, int height){
        this.controller = controller;
        this.width = width;
        this.height = height;

        setupPanel();
    }

    private void setupPanel(){
        layout = new BorderLayout();
        setLayout(layout);

        panelNorth = new PanelNorth(controller,width,height);
        panelSouth = new PanelSouth(controller,width,height);

        Border border = this.getBorder();
        Border margin = BorderFactory.createEmptyBorder(10,10,10,10);
        setBorder(new CompoundBorder(border,margin));

        add(panelNorth,BorderLayout.NORTH);
        add(panelSouth,BorderLayout.SOUTH);
    }


}
